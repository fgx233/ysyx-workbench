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

#include <isa.h>

word_t isa_raise_intr(word_t NO, vaddr_t epc) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * Then return the address of the interrupt/exception vector.
   */
  cpu.mepc = epc;
  cpu.mcause = NO;
  word_t mie_old = BITS(cpu.mstatus, 3,3);
  cpu.mstatus = cpu.mstatus & ~(1 << 7) & ~(1 << 3);
  cpu.mstatus = cpu.mstatus | (mie_old << 7) | (0b11 << 11);
  return cpu.mtvec;
}

void isa_mret_helper() {
  word_t mpie_old = BITS(cpu.mstatus, 7,7);
  cpu.mstatus = ((cpu.mstatus & ~(1 << 3)) | (mpie_old << 3) | (1 << 7)) & ~(0b11 << 11);
}

word_t isa_query_intr() {
  return INTR_EMPTY;
}
