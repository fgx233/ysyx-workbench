#include "common.hpp"
#include "utils.hpp"  
#include "cpu.hpp"
#include "sdb.hpp"
#include <Vtop.h>
#include <verilated_fst_c.h>
#include "Vtop___024root.h"
// 仿真上下文对象指针
VerilatedContext* contextp = nullptr;
// 波形对象指针
VerilatedFstC *tfp = nullptr;
// 电路对象指针
Vtop* top = nullptr;

// 时钟从低电平翻到高电平，再从高电平翻到低电平
void tick() {
  top->clock = 0; top->eval(); tfp->dump(contextp->time()); contextp->timeInc(1);
  top->clock = 1; top->eval(); tfp->dump(contextp->time()); contextp->timeInc(1);
}

// 复位拉高，将时钟滴答十次，再将复位拉低
void reset(int i) {
  top->reset = 1;
  while (i-- > 0)
  {
    tick();
  }
  top->reset = 0;
}

void init_context(const char *wave_file) {
  // 创建仿真上下文对象
  contextp = new VerilatedContext;
  // // 接收仿真上下文参数
  // contextp->commandArgs(argc, argv);
  // 打开波形开关
  contextp->traceEverOn(true);
  // 创建FST波形对象
  tfp = new VerilatedFstC;
  // 创建电路对象
  top = new Vtop{contextp};
  // 设置波形追踪深度
  top->trace(tfp, 99);
  // 打开波形文件
  tfp->open(wave_file);
}

void close_context() {
  // 收尾：结束仿真并释放资源
  top->final();
  tfp->close();
  delete top;
  delete contextp;
}

// npc寄存器数据
riscv32e_NPC_state npc = {};

void assert_fail_msg() {
  isa_reg_display();
}

// 将npc中的寄存器数据提取出来
void npc_sync() {
  auto *r = top->rootp;
  npc.pc = r->top__DOT__pc__DOT__pcReg;

  npc.gpr[0] = r->top__DOT__regs__DOT__regs_0;
  npc.gpr[1] = r->top__DOT__regs__DOT__regs_1;
  npc.gpr[2] = r->top__DOT__regs__DOT__regs_2;
  npc.gpr[3] = r->top__DOT__regs__DOT__regs_3;
  npc.gpr[4] = r->top__DOT__regs__DOT__regs_4;
  npc.gpr[5] = r->top__DOT__regs__DOT__regs_5;
  npc.gpr[6] = r->top__DOT__regs__DOT__regs_6;
  npc.gpr[7] = r->top__DOT__regs__DOT__regs_7;
  npc.gpr[8] = r->top__DOT__regs__DOT__regs_8;
  npc.gpr[9] = r->top__DOT__regs__DOT__regs_9;
  npc.gpr[10] = r->top__DOT__regs__DOT__regs_10;
  npc.gpr[11] = r->top__DOT__regs__DOT__regs_11;
  npc.gpr[12] = r->top__DOT__regs__DOT__regs_12;
  npc.gpr[13] = r->top__DOT__regs__DOT__regs_13;
  npc.gpr[14] = r->top__DOT__regs__DOT__regs_14;
  npc.gpr[15] = r->top__DOT__regs__DOT__regs_15;
}

static void trace_and_difftest() {
  check_wp();
}

// 执行一次npc
static void exec_once() {
  tick();
  npc_sync();
}

// 执行n次npc
static void execute(uint64_t n) {
  for (;n > 0; n --) {
    exec_once();
    trace_and_difftest();
    if (npc_state.state != NPC_RUNNING) break;
  }
}

// 驱动整个npc的接口
void cpu_exec(uint64_t n) {
  switch (npc_state.state) {
    case NPC_END: case NPC_ABORT: case NPC_QUIT:
      printf("Program execution has ended. To restart the program, exit NEMU and run again.\n");
      return;
    default: npc_state.state = NPC_RUNNING;
  }

  execute(n);

  switch (npc_state.state) {
    case NPC_RUNNING: npc_state.state = NPC_STOP; break;

    case NPC_END: case NPC_ABORT:
      Log("npc: %s at pc = " FMT_WORD,
          (npc_state.state == NPC_ABORT ? ANSI_FMT("ABORT", ANSI_FG_RED) :
           (npc_state.halt_ret == 0 ? ANSI_FMT("HIT GOOD TRAP", ANSI_FG_GREEN) :
            ANSI_FMT("HIT BAD TRAP", ANSI_FG_RED))),
          npc_state.halt_pc);
      // fall through
    case NPC_QUIT: ;
  }
}