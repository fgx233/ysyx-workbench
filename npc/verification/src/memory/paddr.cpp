#include "common.hpp"
#include "cpu.hpp"
#include "paddr.hpp"
#include "utils.hpp"

static uint8_t pmem[CONFIG_MSIZE] = {0};

uint8_t* guest_to_host(paddr_t paddr) { return pmem + paddr - CONFIG_MBASE; }
paddr_t host_to_guest(uint8_t *haddr) { return haddr - pmem + CONFIG_MBASE; }


void init_mem() {
  memset(pmem, rand(), CONFIG_MSIZE);
  Log("physical memory area [" FMT_PADDR ", " FMT_PADDR "]", PMEM_LEFT, PMEM_RIGHT);
}


static word_t pmem_read(paddr_t addr, int len) {
  word_t ret = host_read(guest_to_host(addr), len);
  return ret;
}

static void pmem_write(paddr_t addr, int len, word_t data) {
  host_write(guest_to_host(addr), len, data);
}

static void out_of_bound(paddr_t addr) {
  panic("address = " FMT_PADDR " is out of bound of pmem [" FMT_PADDR ", " FMT_PADDR "] at pc = " FMT_WORD,
      addr, PMEM_LEFT, PMEM_RIGHT, npc.pc);
}

word_t paddr_read(paddr_t addr, int len) {
  if (likely(in_pmem(addr))) return pmem_read(addr, len);
  out_of_bound(addr);
  return 0;
}

void paddr_write(paddr_t addr, int len, word_t data) {
  if (likely(in_pmem(addr))) { pmem_write(addr, len, data); return; }
  out_of_bound(addr);
}