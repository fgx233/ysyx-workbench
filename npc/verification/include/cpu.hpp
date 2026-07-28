#ifndef __CPU_H__
#define __CPU_H__

#include "common.hpp"

typedef struct {
  word_t gpr[16];
  paddr_t pc;
} riscv32e_NPC_state;

typedef struct {
  uint32_t inst;
} riscv32_ISADecodeInfo;

typedef struct Decode {
  vaddr_t pc;
  vaddr_t snpc; // static next pc
  vaddr_t dnpc; // dynamic next pc
  riscv32_ISADecodeInfo isa;
  IFDEF(CONFIG_ITRACE, char logbuf[128]);
} Decode;
// cpu-exec.c
void tick();

void reset(int i);

void init_context(const char *wave_file);

void close_context();

extern riscv32e_NPC_state npc;

void npc_sync();

void cpu_exec(uint64_t n);

// init.c
void init_isa();

// logo.c
extern unsigned char isa_logo[];

// hostcall.c
void set_npc_state(int state, paddr_t pc, int halt_ret);
void invalid_inst(paddr_t thispc);

// regs.c
void isa_reg_display();
word_t isa_reg_str2val(const char *s, bool *success);

static inline int check_reg_idx(int idx) {
  IFDEF(CONFIG_RT_CHECK, assert(idx >= 0 && idx < MUXDEF(CONFIG_RVE, 16, 32)));
  return idx;
}

static inline const char* reg_name(int idx) {
  extern const char* regs[];
  return regs[check_reg_idx(idx)];
}
#endif