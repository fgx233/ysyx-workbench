#ifndef __PADDR_H__
#define __PADDR_H__
#include "common.hpp"

#define PMEM_LEFT  ((paddr_t)CONFIG_MBASE)
#define PMEM_RIGHT ((paddr_t)CONFIG_MBASE + CONFIG_MSIZE - 1)
#define RESET_VECTOR (PMEM_LEFT + CONFIG_PC_RESET_OFFSET)

void init_mem();

word_t paddr_read(paddr_t addr, int len);

void paddr_write(paddr_t addr, int len, word_t data);

static inline word_t host_read(void *addr, int len) {
  switch (len) {
    case 1: return *(uint8_t  *)addr;
    case 2: return *(uint16_t *)addr;
    case 4: return *(uint32_t *)addr;
    default: return 0;
  }
}

static inline void host_write(void *addr, int len, word_t data) {
  switch (len) {
    case 1: *(uint8_t  *)addr = data; return;
    case 2: *(uint16_t *)addr = data; return;
    case 4: *(uint32_t *)addr = data; return;
  }
}

static inline bool in_pmem(paddr_t addr) {
  return addr - CONFIG_MBASE < CONFIG_MSIZE;
}

// 把物理地址转换为指向pmem大数组的指针
uint8_t* guest_to_host(paddr_t paddr);
// 把指向大数组的指针转换为物理地址
paddr_t host_to_guest(uint8_t *haddr);

#endif