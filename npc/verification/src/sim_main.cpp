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

typedef struct info {
  uint8_t input;
  uint8_t output;
  uint8_t flag;
} info;

void encoder(info& d) {
  uint8_t out = 0;
  for (uint8_t i = 7; i > 0; i --) {
    if ((d.input >> i) & 1) {
      out = i;
      break;
    }
  }
  d.output = out;
  d.flag = d.input != 0;
}

int main(int argc, char** argv) {
  // 仿真上下文：管理仿真时间、命令行参数等
  VerilatedContext* contextp = new VerilatedContext;
  contextp->commandArgs(argc, argv);
  contextp->traceEverOn(true);
  VerilatedFstC *tfp = new VerilatedFstC;
  // 实例化被测模块
  Vtop* top = new Vtop{contextp};

  top->trace(tfp, 99);
  tfp->open("build/wave.fst");

  int total = 0;
  int right = 0;
  for (uint8_t i = 0; i <= 255; i ++) {

    total ++;
    top->io_in = i;
    info d;
    d.input = i;
    encoder(d);

    top->eval();
    tfp->dump(contextp->time());
    contextp->timeInc(1);

    if (d.output == top->io_out && d.flag == top->io_flag) {
      right ++;
    } else {
      std::cout << "输入：" << std::bitset<8>(i) << "参考模型 ：out = " << (int)d.output << " flag = " << (int)d.flag <<
      "电路： out = " << (int)top->io_out << " flag = " << (int)top->io_flag << std::endl;
    }

    if (i == 255) {
      break;
    }
  }

  printf("总测试数%d次，正确%d次，错误%d次，正确率%f\n", total, right, total-right, (float)right/total);

  // 收尾：结束仿真并释放资源
  top->final();
  tfp->close();
  delete top;
  delete contextp;
  return 0;
}