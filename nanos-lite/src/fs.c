#include <fs.h>

size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t ramdisk_write(const void *buf, size_t offset, size_t len);

size_t serial_write(const void *buf, size_t offset, size_t len);

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  ReadFn read;
  WriteFn write;
  uintptr_t pos;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]  = {"stdin", 0, 0, invalid_read, invalid_write},
  [FD_STDOUT] = {"stdout", 0, 0, invalid_read, serial_write},
  [FD_STDERR] = {"stderr", 0, 0, invalid_read, serial_write},
#include "files.h"
};

char *fd_name(int fd) {
  return file_table[fd].name;
}

int fs_open(const char *pathname, int flags, int mode) {
  int len = LENGTH(file_table);
  for (int i = 0; i < len; i ++) {
    if (strcmp(pathname, file_table[i].name) == 0) {
      file_table[i].pos = 0;
      return i;
    }
  }
  panic("没有找到这个文件：%s", pathname);
}

size_t fs_read(int fd, void *buf, size_t len) {
  if (fd == FD_STDIN || fd == FD_STDOUT || fd == FD_STDERR) {
    return 0;
  }

  if (len + file_table[fd].pos > file_table[fd].size) {
    len = file_table[fd].size - file_table[fd].pos;
  }

  uintptr_t pos = file_table[fd].pos;
  file_table[fd].pos += len;
  return ramdisk_read(buf, file_table[fd].disk_offset + pos, len);
}

size_t fs_write(int fd, const void *buf, size_t len) {
  if (fd == FD_STDIN) {
    return 0;
  }
  if (fd == FD_STDOUT || fd == FD_STDERR) {
    return serial_write(buf, 0, len);
  }
  
  if (len + file_table[fd].pos > file_table[fd].size) {
    len = file_table[fd].size - file_table[fd].pos;
  }
  uintptr_t pos = file_table[fd].pos;
  file_table[fd].pos += len;
  return ramdisk_write(buf, file_table[fd].disk_offset + pos, len);
}

size_t fs_lseek(int fd, size_t offset, int whence) {
  uintptr_t new_addr = 0;
  if (whence == SEEK_SET) {
    new_addr = offset;
  } else if (whence == SEEK_CUR) {
    new_addr = file_table[fd].pos + offset;
  } else if (whence == SEEK_END) {
    new_addr = file_table[fd].size + offset;
  } else {
    panic("错误的whence：%d", whence);
  }
  if (new_addr > file_table[fd].size) {
    return -1;
  }

  file_table[fd].pos = new_addr;
  return new_addr;
}

int fs_close(int fd) {
  return 0;
}

void init_fs() {
  // TODO: initialize the size of /dev/fb
}
