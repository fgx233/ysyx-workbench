/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
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

#include <common.h>

void init_monitor(int, char *[]);
void am_init_monitor();
void engine_start();
int is_exit_status_bad();

word_t expr(char *e, bool *success);

int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
#ifdef CONFIG_TARGET_AM
  am_init_monitor();
#else
  init_monitor(argc, argv);
#endif
  // FILE *input = fopen("/home/fgx/Desktop/ysyx-workbench/nemu/tools/gen-expr/input", "r");
  // if(input == NULL)
  // {
  //   Assert(0, "测试文件不存在");
  // }
  // double good = 0;
  // double bad = 0;
  // char line[256] = {0};
  // while (fgets(line, sizeof(line), input) != NULL)
  // {
  //   uint32_t expected_val;
  //   char expression[256];

  //   sscanf(line, "%d %[^\n]", &expected_val, expression);
  //   bool successful = false;
  //   int result = expr(expression, &successful);
  //   if(successful == false)
  //   {
  //     printf("表达式%s不合法\n", expression);
  //     continue;
  //   }
  //   else
  //   {
  //     // printf("表达式：%s, 计算值：%u, 预期值：%u\n", expression, result, expected_val);
  //     if(result == expected_val)
  //     {
  //       good++;
  //     }
  //     else{
  //       bad++;
  //       printf("计算错误：%s, expected = %u, result = %u\n", expression, expected_val, result);
  //       break;
  //     }

  //   }
  // }
  
  // printf("good = %lf, bad = %lf, 计算正确率：%lf%%\n",good, bad, 100*(good)/(good + bad));
  
  // fclose(input);


  /* Start engine. */
  engine_start();

  return is_exit_status_bad();
}
