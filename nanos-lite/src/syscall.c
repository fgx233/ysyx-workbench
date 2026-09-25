#include <common.h>
#include "syscall.h"

IFDEF(CONFIG_STRACE, static char *str[] = {
  "SYS_exit",
  "SYS_yield",
  "SYS_open",
  "SYS_read",
  "SYS_write",
  "SYS_kill",
  "SYS_getpid",
  "SYS_close",
  "SYS_lseek",
  "SYS_brk",
  "SYS_fstat",
  "SYS_time",
  "SYS_signal",
  "SYS_execve",
  "SYS_fork",
  "SYS_link",
  "SYS_unlink",
  "SYS_wait",
  "SYS_times",
  "SYS_gettimeofday"
});

uint32_t sys_yield() {
  yield();
  return 0;
}

void sys_exit(uintptr_t status) {
  halt(status);
}

intptr_t sys_write(int fd, const void *buf, size_t count) {
  if (fd != 1 && fd != 2) {
    panic("不合法的输出接口：%d", fd);
  }
  int ret = 0;
  const char *p = buf;
  for (int i = 0; i < count; i ++) {
    putch(*p);
    p++;
    ret += 1;
  }
  return ret;
}

uintptr_t sys_brk(uintptr_t addr) {
  return 0;
}

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;
  IFDEF(CONFIG_STRACE, Log("STRACE: %s a0(%x) a1(%x) a2(%x)", str[a[0]], a[1], a[2], a[3]));
  switch (a[0]) {
    case SYS_exit : sys_exit(a[1]); break;
    case SYS_yield: c->GPRx = sys_yield(); break;
    case SYS_write: c->GPRx = sys_write(a[1], (const void *)a[2], a[3]); break;
    case SYS_brk:   c->GPRx = sys_brk(a[1]); break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
  
}

