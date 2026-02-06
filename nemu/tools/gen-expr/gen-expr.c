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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

#define MAX 65536

// this should be enough
static char buf[MAX] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

int total_size = 0;
int deep = 0;


static uint32_t choose(int x) {
  return (uint32_t)(rand() % x);
}

static void gen_num() {
  uint32_t num = rand();

  int n = snprintf(buf + total_size, MAX - total_size, "%u", num);
  total_size += n;
  if (total_size >= MAX) {
    printf("超出了数组最大长度\n");
  }
}

static void gen_rand_op() {
  char op;
  switch (choose(4))
  {
  case 0: op = '+';break;
  case 1: op = '-';break;
  case 2: op = '*';break;
  default:op = '/';break;
  }

  int n = snprintf(buf + total_size, MAX - total_size, "%c", op);
  total_size += n;
  if (total_size >= MAX) {
    printf("超出了数组最大长度\n");
  }
}

static void gen(char a) {
  int n = snprintf(buf + total_size, MAX - total_size, "%c", a);
  total_size += n;
  if (total_size >= MAX) {
    printf("超出了数组最大长度\n");
  }
}

static void gen_rand_expr() {
  deep++;
  if (deep >= 7) {
    gen_num();
    return;
  }
  switch (choose(3))
  {
  case 0: gen_num(); break;
  case 1: gen('('); gen_rand_expr(); gen(')'); break;
  default:gen_rand_expr(); gen_rand_op(); gen_rand_expr(); break;
  }
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);

  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {

    total_size = 0;
    deep = 0;
    gen_rand_expr();

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}
