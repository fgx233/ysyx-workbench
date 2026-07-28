#include "common.hpp"
#include "paddr.hpp"
#include "utils.hpp"  
#include "cpu.hpp"
#include "sdb.hpp"
#include <Vtop.h>
#include <verilated_fst_c.h>
#include "Vtop___024root.h"

#define MAX_INST_TO_PRINT 10

uint64_t g_nr_guest_inst = 0;
static uint64_t g_timer = 0; // unit: us
static bool g_print_step = false;


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

static void statistic() {
  IFNDEF(CONFIG_TARGET_AM, setlocale(LC_NUMERIC, ""));
#define NUMBERIC_FMT MUXDEF(CONFIG_TARGET_AM, "%", "%'") PRIu64
  Log("host time spent = " NUMBERIC_FMT " us", g_timer);
  Log("total guest instructions = " NUMBERIC_FMT, g_nr_guest_inst);
  if (g_timer > 0) Log("simulation frequency = " NUMBERIC_FMT " inst/s", g_nr_guest_inst * 1000000 / g_timer);
  else Log("Finish running in less than 1 us and can not calculate the simulation frequency");
}

void assert_fail_msg() {
  isa_reg_display();
  IFDEF(CONFIG_IRINGBUF, iringbuf_print());
  statistic();
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

static void trace_and_difftest(Decode *_this) {
#ifdef CONFIG_ITRACE_COND
  if (ITRACE_COND) { log_write("%s\n", _this->logbuf); }
#endif
  if (g_print_step) { IFDEF(CONFIG_ITRACE, puts(_this->logbuf)); }
  IFDEF(CONFIG_IRINGBUF, iringbuf_insert(_this->logbuf));
  IFDEF(CONFIG_DIFFTEST, difftest_step(_this->pc));
  IFDEF(CONFIG_WATCH_POINT, check_wp());
}

// 执行一次npc
static void exec_once(Decode *s, vaddr_t pc) {
  s->pc = pc;
  s->snpc = pc + 4;

  s->isa.inst = paddr_read(pc & ~(0b11), 4);

  tick();
  npc_sync();

  s->dnpc = npc.pc;

  IFDEF(CONFIG_FTRACE, ftrace_check(s));

#ifdef CONFIG_ITRACE
  char *p = s->logbuf;
  p += snprintf(p, sizeof(s->logbuf), FMT_WORD ":", s->pc);
  int ilen = s->snpc - s->pc;
  int i;
  uint8_t *inst = (uint8_t *)&s->isa.inst;

  for (i = ilen - 1; i >= 0; i --) {
    p += snprintf(p, 4, " %02x", inst[i]);
  }
  int ilen_max = 4;
  int space_len = ilen_max - ilen;
  if (space_len < 0) space_len = 0;
  space_len = space_len * 3 + 1;
  memset(p, ' ', space_len);
  p += space_len;

  void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte);
  disassemble(p, s->logbuf + sizeof(s->logbuf) - p, s->pc, (uint8_t *)&s->isa.inst, ilen);
#endif
}

// 执行n次npc
static void execute(uint64_t n) {
  Decode s;
  for (;n > 0; n --) {
    exec_once(&s, npc.pc);
    g_nr_guest_inst ++;
    trace_and_difftest(&s);
    if (npc_state.state != NPC_RUNNING) break;
  }
}

// 驱动整个npc的接口
void cpu_exec(uint64_t n) {
  g_print_step = (n < MAX_INST_TO_PRINT);
  switch (npc_state.state) {
    case NPC_END: case NPC_ABORT: case NPC_QUIT:
      printf("Program execution has ended. To restart the program, exit NEMU and run again.\n");
      return;
    default: npc_state.state = NPC_RUNNING;
  }

  uint64_t timer_start = get_time();

  execute(n);

  uint64_t timer_end = get_time();
  g_timer += timer_end - timer_start;

  switch (npc_state.state) {
    case NPC_RUNNING: npc_state.state = NPC_STOP; break;

    case NPC_END: case NPC_ABORT:
      Log("npc: %s at pc = " FMT_WORD,
          (npc_state.state == NPC_ABORT ? ANSI_FMT("ABORT", ANSI_FG_RED) :
           (npc_state.halt_ret == 0 ? ANSI_FMT("HIT GOOD TRAP", ANSI_FG_GREEN) :
            ANSI_FMT("HIT BAD TRAP", ANSI_FG_RED))),
          npc_state.halt_pc);
      // fall through
    case NPC_QUIT: statistic();
  }
}