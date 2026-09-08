/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#ifndef __RISCV_REG_H__
#define __RISCV_REG_H__

#include <isa.h>

static inline int check_reg_idx(int idx) {
  IFDEF(CONFIG_RT_CHECK, assert(idx >= 0 && idx < MUXDEF(CONFIG_RVE, 16, 32)));
  return idx;
}

#define gpr(idx) (cpu.gpr[check_reg_idx(idx)])

static inline word_t get_csr(uint32_t idx) {
  switch (idx) {
    case 0x300: return cpu.mstatus; break;
    case 0x305: return cpu.mtvec;   break;
    case 0x341: return cpu.mepc;    break;
    case 0x342: return cpu.mcause;  break;
    default: panic("未知的csr寄存器编号：%u", idx); 
  }
}

static inline void set_csr(uint32_t idx, word_t data) {
  switch (idx) {
    case 0x300: cpu.mstatus = data; break;
    case 0x305: cpu.mtvec   = data; break;
    case 0x341: cpu.mepc    = data; break;
    case 0x342: cpu.mcause  = data; break;
    default: panic("未知的csr寄存器编号：%u", idx); 
  }
}

static inline const char* reg_name(int idx) {
  extern const char* regs[];
  return regs[check_reg_idx(idx)];
}

#endif
