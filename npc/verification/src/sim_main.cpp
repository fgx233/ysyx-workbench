#include <Vtop.h>
#include <verilated_fst_c.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>
#include <bitset>

#define BITMASK(bits) ((1ull << (bits)) - 1)
#define BITS(x, hi, lo) (((x) >> (lo)) & BITMASK((hi) - (lo) + 1)) // similar to x[hi:lo] in verilog
#define SEXT(x, len) ({ struct { int64_t n : len; } __x = { .n = x }; (uint64_t)__x.n; })

VerilatedContext* contextp = nullptr;
VerilatedFstC *tfp = nullptr;
Vtop* top = nullptr;

void tick() {
  top->clock = 0; top->eval(); tfp->dump(contextp->time()); contextp->timeInc(1);
  top->clock = 1; top->eval(); tfp->dump(contextp->time()); contextp->timeInc(1);
}

void reset(int i) {
  top->reset = 1;
  while (i-- > 0)
  {
    tick();
  }
  top->reset = 0;
}

int main(int argc, char** argv) {
  // 仿真上下文：管理仿真时间、命令行参数等
  contextp = new VerilatedContext;
  contextp->commandArgs(argc, argv);
  contextp->traceEverOn(true);
  tfp = new VerilatedFstC;
  // 实例化被测模块
  top = new Vtop{contextp};

  top->trace(tfp, 99);
  tfp->open("build/wave.fst");
  
  reset(10);

  for (int i = 0; i < 100; i ++) {
    tick();
  }
  
  // 收尾：结束仿真并释放资源
  top->final();
  tfp->close();
  delete top;
  delete contextp;
  return 0;
}