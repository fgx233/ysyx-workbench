#include "common.hpp"
#include "cpu.hpp"
#include "paddr.hpp"

// this is not consistent with uint8_t
// but it is ok since we do not access the array directly
static const uint32_t img [] = {
  0x00000297,  // auipc t0,0
  0x00028823,  // sb  zero,16(t0)
  0x0102c503,  // lbu a0,16(t0)
  0x00100073,  // ebreak (used as npc_trap)
  0xdeadbeef,  // some data
};

static void restart() {
  // 复位10个时钟周期
  reset(10);
}

void init_isa() {
  // 加载内置程序
  memcpy(guest_to_host(RESET_VECTOR), img, sizeof(img));

  // 复位cpu
  restart();

  // 复制寄存器状态
  npc_sync();
}