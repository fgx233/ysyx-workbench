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

#include <dlfcn.h>
#include "common.hpp"
#include <cpu.hpp>
#include <paddr.hpp>
#include <utils.hpp>

enum { DIFFTEST_TO_DUT, DIFFTEST_TO_REF };

#define RISCV_GPR_TYPE uint32_t
#define RISCV_GPR_NUM  16
#define DIFFTEST_REG_SIZE (sizeof(RISCV_GPR_TYPE) * (RISCV_GPR_NUM + 1)) // GPRs + pc

void (*ref_difftest_memcpy)(paddr_t addr, void *buf, size_t n, bool direction) = NULL;
void (*ref_difftest_regcpy)(void *dut, bool direction) = NULL;
void (*ref_difftest_exec)(uint64_t n) = NULL;
void (*ref_difftest_raise_intr)(uint64_t NO) = NULL;

#ifdef CONFIG_DIFFTEST

static bool is_skip_ref = false;
static bool is_skip_ref_next = false;

static inline bool difftest_check_reg(const char *name, vaddr_t pc, word_t ref, word_t dut) {
  if (ref != dut) {
    Log("%s is different after executing instruction at pc = " FMT_WORD
        ", right = " FMT_WORD ", wrong = " FMT_WORD ", diff = " FMT_WORD,
        name, pc, ref, dut, ref ^ dut);
    return false;
  }
  return true;
}

bool isa_difftest_checkregs(riscv32e_NPC_state *ref_r, vaddr_t pc) {
  bool is_right = true;
  for (int i = 0; i < MUXDEF(CONFIG_RVE, 16, 32); i ++) {
    if (!difftest_check_reg(reg_name(i), pc, ref_r->gpr[i], npc.gpr[i])) {
      is_right = false;
    }
  }
  if (!difftest_check_reg("pc", pc, ref_r->pc, npc.pc)) {
    is_right = false;
  }

  return is_right;
}

// this is used to let ref skip instructions which
// can not produce consistent behavior with NEMU
void difftest_skip_ref() {
  is_skip_ref = true;
}

void difftest_skip_ref_next() {
  is_skip_ref_next = true;
}




void init_difftest(char *ref_so_file, long img_size, int port) {
  assert(ref_so_file != NULL);

  void *handle;
  handle = dlopen(ref_so_file, RTLD_LAZY);
  assert(handle);

  ref_difftest_memcpy = (void (*)(paddr_t addr, void *buf, size_t n, bool direction))dlsym(handle, "difftest_memcpy");
  assert(ref_difftest_memcpy);

  ref_difftest_regcpy = (void (*)(void *dut, bool direction))dlsym(handle, "difftest_regcpy");
  assert(ref_difftest_regcpy);

  ref_difftest_exec = (void (*)(uint64_t n))dlsym(handle, "difftest_exec");
  assert(ref_difftest_exec);

  ref_difftest_raise_intr = (void (*)(uint64_t NO))dlsym(handle, "difftest_raise_intr");
  assert(ref_difftest_raise_intr);

  void (*ref_difftest_init)(int) = (void (*)(int))dlsym(handle, "difftest_init");
  assert(ref_difftest_init);

  Log("Differential testing: %s", ANSI_FMT("ON", ANSI_FG_GREEN));
  Log("The result of every instruction will be compared with %s. "
      "This will help you a lot for debugging, but also significantly reduce the performance. "
      "If it is not necessary, you can turn it off in menuconfig.", ref_so_file);

  ref_difftest_init(port);
  ref_difftest_memcpy(RESET_VECTOR, guest_to_host(RESET_VECTOR), img_size, DIFFTEST_TO_REF);
  ref_difftest_regcpy(&npc, DIFFTEST_TO_REF);
}

static void checkregs(riscv32e_NPC_state *ref, vaddr_t pc) {
  if (!isa_difftest_checkregs(ref, pc)) {
    npc_state.state = NPC_ABORT;
    npc_state.halt_pc = pc;
    isa_reg_display();
  }
}

void difftest_step(vaddr_t pc) {
  riscv32e_NPC_state ref_r;

  // if (is_skip_ref) {
  //   ref_difftest_regcpy(&npc, DIFFTEST_TO_REF);
  //   is_skip_ref = false;
  //   return;
  // }

  if (is_skip_ref || is_skip_ref_next) {
    if (is_skip_ref && is_skip_ref_next) {
      ref_difftest_regcpy(&npc, DIFFTEST_TO_REF);
      is_skip_ref_next = false;
      return;
    } else if (is_skip_ref) {
      ref_difftest_regcpy(&npc, DIFFTEST_TO_REF);
      is_skip_ref = false;
      return;
    } else {
      is_skip_ref_next = false;
      is_skip_ref = true;
    }

  }

  ref_difftest_exec(1);
  ref_difftest_regcpy(&ref_r, DIFFTEST_TO_DUT);

  checkregs(&ref_r, pc);
}
#else
void init_difftest(char *ref_so_file, long img_size, int port) { }
#endif
