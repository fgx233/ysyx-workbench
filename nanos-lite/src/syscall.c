#include <common.h>
#include "syscall.h"

static char *str[] = {
  "SYS_exit",
  "SYS_yield"
};

uint32_t sys_yield() {
  yield();
  return 0;
}

void sys_exit(uintptr_t status) {
  halt(status);
}

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;
  switch (a[0]) {
    case SYS_exit : sys_exit(a[1]); break;
    case SYS_yield: c->GPRx = sys_yield(); break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
  Log("STRACE: %s a0(%x) a1(%x) a2(%x) ret(%x)", str[a[0]], a[1], a[2], a[3], c->GPRx);
}

