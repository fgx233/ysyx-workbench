#include "common.hpp"
#include "paddr.hpp"
#include "cpu.hpp"
#include "Vtop__Dpi.h"

extern "C" void ebreak(int pc) {
  set_npc_state(NPC_END, (vaddr_t)pc, npc.gpr[10]);
}

extern "C" void inv(int pc) {
  invalid_inst((vaddr_t)pc);
}

void set_npc_state(int state, paddr_t pc, int halt_ret) {
  npc_state.state = state;
  npc_state.halt_pc = pc;
  npc_state.halt_ret = halt_ret;
}

void invalid_inst(paddr_t thispc) {
  uint32_t temp[2];
  paddr_t pc = thispc;
  vaddr_ifetch((int)pc, (int *)(&temp[0]));
  vaddr_ifetch((int)(pc + 4), (int *)(&temp[1]));

  uint8_t *p = (uint8_t *)temp;
  printf("invalid opcode(PC = " FMT_WORD "):\n"
      "\t%02x %02x %02x %02x %02x %02x %02x %02x ...\n"
      "\t%08x %08x...\n",
      thispc, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], temp[0], temp[1]);

  printf("There are two cases which will trigger this unexpected exception:\n"
      "1. The instruction at PC = " FMT_WORD " is not implemented.\n"
      "2. Something is implemented incorrectly.\n", thispc);
  printf("Find this PC(" FMT_WORD ") in the disassembling result to distinguish which case it is.\n\n", thispc);
  printf(ANSI_FMT("If it is the first case, see\n%s\nfor more details.\n\n"
        "If it is the second case, remember:\n"
        "* The machine is always right!\n"
        "* Every line of untested code is always wrong!\n\n", ANSI_FG_RED), isa_logo);

  set_npc_state(NPC_ABORT, thispc, -1);
}
