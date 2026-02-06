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
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args) {
  if (args == NULL) {       //若没有参数，默认执行一条指令
    cpu_exec(1);
    return 0;
  }

  int n = 0;
  int num = sscanf(args, "%d", &n);

  if (num != 1 || n < 0) {
    printf("参数：“%s”不合法。\n", args);
    return 0;
  }

  cpu_exec(n);
  return 0;
}

static int cmd_info(char *args) {
  if (args == NULL) {
    printf("请输入要查看的对象，info r/w。\n");
    return 0;
  }

  char opt;
  if (sscanf(args, "%c", &opt) != 1) {
    printf("参数读取错误，请重新输入\n");
    return 0;
  }

  if (opt == 'r') {
    isa_reg_display();
    return 0;
  } else if (opt == 'w') {
    //待实现监视点
    return 0;
  } else {
    printf("错误的参数：%s\n", args);
    return 0;
  }
}

static int cmd_x(char *args) {

  if (args == NULL) {
    printf("无有效输入，请重新输入地址表达式\n");
    return 0;
  }
  
  int n = 0;
  paddr_t expr = 0;

  if (sscanf(args, "%d" " " SCN_PADDR, &n, &expr) != 2) {
    printf("错误的参数：%s\n", args);
    return 0;
  }

  for (int i = 0; i < n; i++) {
    printf(FMT_PADDR ": " FMT_WORD "\n", expr, paddr_read(expr, 4));
    expr += 4;
  }
  return 0;
}

static int cmd_p(char *args) {

  if (args == NULL) {
    printf("无有效输入，请重新输入待求值表达式\n");
    return 0;
  }

  bool success = true;
  word_t res = expr(args, &success);
  if (success == false) {
    printf("运算出现错误，请重新检查输入\n");
    return 0;
  } else {
    printf("计算结果是:" FMT_SWORD "\n", res);
    return 0;
  }

}

static int cmd_test(char *args) {
  FILE *fp = fopen("/home/fgx/ysyx-workbench/nemu/test/gen-expr/input", "r");
  if (fp == NULL) {
    printf("打开测试文件失败，请检查文件是否存在\n");
    return 0;
  }

  char str[256] = {0};

  int total_check = 0;
  int err_num = 0;

  while (fgets(str, sizeof(str), fp) != NULL)
  {
    total_check++;

    word_t expect_result = 0;
    char expression[256] = {0};

    sscanf(str, "%u %s", &expect_result, expression);
    bool success = true;
    word_t my_result = expr(expression, &success);

    if (success == false) {
      err_num++;
      printf("求值出错：%s\n", expression);
      printf("------------------------------------------\n");
    } else if (my_result != expect_result) {
      err_num++;
      printf("错误表达式：%s\n", expression);
      printf("期望值：" FMT_SWORD "\n", expect_result);
      printf("实际值：" FMT_SWORD "\n", my_result);
      printf("------------------------------------------\n");
    }

  }
  
  printf("计算正确率：%f%%\n", 100*((total_check - err_num)/(double)total_check));

  fclose(fp);

  return 0;
}

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "test", "test the eval function use input file", cmd_test},
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "execute N steps, default: N = 1", cmd_si },
  { "info", "display register(r) or watchpoint(w)", cmd_info },
  { "x", "scan the N words form given expr in memory, usage: x N expr", cmd_x},
  { "p", "calculate the expr", cmd_p},

  /* TODO: Add more commands */

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}



void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
