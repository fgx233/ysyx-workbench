#include <common.h>
#include "syscall.h"
#include "fs.h" 
#include <sys/time.h>

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


uintptr_t sys_brk(uintptr_t addr) {
  return 0;
}

int sys_gettimeofday(struct timeval *tv, struct timezone *tz) {
  uint64_t us = io_read(AM_TIMER_UPTIME).us;
  uint32_t sec = us / 1000000;
  uint32_t usec = us % 1000000;
  
  tv->tv_sec = sec;
  tv->tv_usec = usec;

  return 0;
}

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;

#ifdef CONFIG_STRACE
  switch (a[0]) {
    case SYS_open:
             Log("STRACE: %s file_name(%s) a1(%x) a2(%x)", str[a[0]], a[1], a[2], a[3]);break;
    case SYS_read:
    case SYS_write:
    case SYS_lseek:
    case SYS_close:
             Log("STRACE: %s file_name(%s) a1(%x) a2(%x)", str[a[0]], fd_name(a[1]), a[2], a[3]);break;
    default: Log("STRACE: %s a0(%x) a1(%x) a2(%x)", str[a[0]], a[1], a[2], a[3]);break;
  }
#endif

  switch (a[0]) {
    case SYS_exit : sys_exit(a[1]); break;
    case SYS_yield: c->GPRx = sys_yield(); break;
    case SYS_open:  c->GPRx = fs_open((const char *)a[1], a[2], a[3]); break;
    case SYS_read:  c->GPRx = fs_read(a[1], (void *)a[2], a[3]); break;
    case SYS_write: c->GPRx = fs_write(a[1], (const void *)a[2], a[3]); break;
    case SYS_lseek: c->GPRx = fs_lseek(a[1], a[2], a[3]); break;
    case SYS_close: c->GPRx = fs_close(a[1]); break;
    case SYS_gettimeofday: c->GPRx = sys_gettimeofday((struct timeval *)a[1], (struct timezone *)a[2]); break;
    case SYS_brk:   c->GPRx = sys_brk(a[1]); break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
  
}

