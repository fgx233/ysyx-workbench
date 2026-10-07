#include <proc.h>
#include <elf.h>
#include <fs.h>

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t ramdisk_write(const void *buf, size_t offset, size_t len);

static uintptr_t loader(PCB *pcb, const char *filename) {
  // // 1、读elf头
  // Elf32_Ehdr elf_head;      
  // ramdisk_read(&elf_head, 0, sizeof(Elf32_Ehdr));
  // assert(*(uint32_t *)elf_head.e_ident == 0x464C457F);
  // // 2、找到程序头表
  // Elf32_Off phoff = elf_head.e_phoff;
  // Elf32_Half phentsize = elf_head.e_phentsize;
  // Elf32_Half phnum = elf_head.e_phnum;
  // // 3、提取程序头表
  // Elf32_Phdr phdr_table[phnum];
  // ramdisk_read(&phdr_table, phoff, phentsize * phnum);
  // for (int i = 0; i < phnum; i ++) {
  //   if (phdr_table[i].p_type == PT_LOAD) {
  //     Elf32_Off offset = phdr_table[i].p_offset;
  //     Elf32_Addr vaddr = phdr_table[i].p_vaddr;
  //     Elf32_Word filesz = phdr_table[i].p_filesz;
  //     Elf32_Word memsz = phdr_table[i].p_memsz;

  //     ramdisk_read((void *)vaddr, offset, filesz);
  //     memset((void *)(vaddr + filesz), 0, memsz - filesz);
  //   }
  // }
  // return elf_head.e_entry;
  int fd = fs_open(filename, 0, 0);
  Elf32_Ehdr elf_head;
  fs_read(fd, &elf_head, sizeof(Elf32_Ehdr));
  if (fd == -1) {
    panic("没找到这个文件：%s", filename);
  }

  assert(*(uint32_t *)elf_head.e_ident == 0x464C457F);

  Elf32_Off phoff = elf_head.e_phoff;
  Elf32_Half phentsize = elf_head.e_phentsize;
  Elf32_Half phnum = elf_head.e_phnum;

  Elf32_Phdr phdr_table[phnum];
  fs_lseek(fd, phoff, SEEK_SET);
  fs_read(fd, &phdr_table, phentsize * phnum);
  for (int i = 0; i < phnum; i ++) {
    if (phdr_table[i].p_type == PT_LOAD) {
      Elf32_Off offset = phdr_table[i].p_offset;
      Elf32_Addr vaddr = phdr_table[i].p_vaddr;
      Elf32_Word filesz = phdr_table[i].p_filesz;
      Elf32_Word memsz = phdr_table[i].p_memsz;
      fs_lseek(fd, offset, SEEK_SET);
      fs_read(fd, (void *)vaddr, filesz);
      memset((void *)(vaddr + filesz), 0, memsz - filesz);
    }
  }
  fs_close(fd);
  return elf_head.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p, filename = %s", entry, filename);
  ((void(*)())entry) ();
}

