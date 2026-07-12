#include "common.hpp"
#include "paddr.hpp"
#include "Vtop__Dpi.h"

extern "C" void vaddr_ifetch(int inst_addr, int *inst) {
  *inst = paddr_read((vaddr_t)inst_addr & ~(0b11), 4);
}

extern "C" void vaddr_read(int raddr, int *rdata) {
  word_t ret = paddr_read((vaddr_t)raddr & ~(0b11), 4);
  *rdata = ret;
}

extern "C" void vaddr_write(int waddr, int wdata, char wmask) {
  vaddr_t fixed_addr = (vaddr_t)waddr & ~(0b11); 
  word_t fixed_data = (word_t)wdata;
  word_t fixed_wmask = (uint8_t)wmask;
  switch(fixed_wmask & 0xF) {
    case 0xF:   paddr_write(fixed_addr, 4, fixed_data); break;
    case 0b0001:paddr_write(fixed_addr, 1, fixed_data); break;
    case 0b0010:paddr_write(fixed_addr + 1, 1, fixed_data); break;
    case 0b0100:paddr_write(fixed_addr + 2, 1, fixed_data); break;
    case 0b1000:paddr_write(fixed_addr + 3, 1, fixed_data); break;
    default:    paddr_write(fixed_addr, 4, fixed_data); break;
  }
}