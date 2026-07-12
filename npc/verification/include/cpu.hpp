#ifndef __CPU_H__
#define __CPU_H__

#include "common.hpp"

typedef struct {
  word_t gpr[16];
  paddr_t pc;
} riscv32e_NPC_state;

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
#endif