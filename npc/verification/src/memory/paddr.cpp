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
  
  if (addr - CONFIG_RTC_MMIO < 8) {
    static uint32_t rtc[2] = {0};
    int offset = addr - CONFIG_RTC_MMIO;
    Assert(offset == 0 || offset == 4, "时钟读偏移错误：offset=%d", offset);
    if (offset == 4) {
      uint64_t us = get_time();
      rtc[0] = (uint32_t)us;
      rtc[1] = us >> 32;
    }
    if (offset == 4) {
      return rtc[1];
    } else {
      return rtc[0];
    }
  }

  out_of_bound(addr);
  return 0;
}

void paddr_write(paddr_t addr, int len, word_t data) {
  if (likely(in_pmem(addr))) { pmem_write(addr, len, data); return; }

  if (addr == CONFIG_SERIAL_MMIO) {
    Assert(len == 1, "串口写长度错误，len=%d", len);
    putc(uint8_t(data), stderr);
    return;
  }

  out_of_bound(addr);
}