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

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_DEC,

  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces   空格串（一个或多个空格）
  {"0|[1-9][0-9]*", TK_DEC},   //          十进制整数
  {"\\+", '+'},         // plus     +
  {"\\-", '-'},         //          -
  {"\\*", '*'},         //          *
  {"\\/", '/'},         //          /
  {"\\(", '('},         //          (
  {"\\)", ')'},         //          )
  {"==", TK_EQ},        // equal    ==
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
        

        if (rules[i].token_type != TK_NOTYPE) {
          tokens[nr_token].type = rules[i].token_type;//将匹配到的字符串类型写入对应的token元素的类型
          int nr_write = snprintf(tokens[nr_token].str, sizeof(tokens[nr_token].str), "%.*s", substr_len, substr_start);//将匹配到的字符串全部写入token元素的字符串
          if (nr_write >= sizeof(tokens[i].str) || nr_write < 0) {      //token读取正确性检测
            printf("token有错误或过长:%.*s\n", substr_len, substr_start);
            return false;
          }
          nr_token++;
          break;
        }        
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

bool check_parentheses(int p, int q) {
    int nr_brace = 0;

    if (tokens[p].type == '(' && tokens[q].type == ')') {
        for (int i = p; i <= q; i ++) {
            if (tokens[i].type == '(') {
                nr_brace += 1;
            }
            if (tokens[i].type == ')') {
                nr_brace -= 1;
            }
            if (nr_brace < 0 || (nr_brace == 0 && i != q)) {
                return false;
            }
        }
        
        if (nr_brace == 0) {
            return true;
        }
        return false;
    }

    return false;
}

int find_op(int p, int q) {
  int op = p;
  int old_class = 100;
  int new_class = 0;
  int jump = 0;
  for(int i = p; i < q; i++) {

    if (tokens[i].type == '(')
    {
      jump += 1;
      continue;
    } else if (tokens[i].type == ')') {
      jump -= 1;
      continue;
    } 

    if (jump != 0) {
      continue;
    }
    
    switch (tokens[i].type)
    {
    case '+': case '-': new_class = 1;break;
    case '*': case '/': new_class = 2;break;
    default: new_class = 100;
    }

    if (old_class >= new_class) {
      old_class = new_class;
      op = i;
    }
  }
  return op;
}

word_t eval(int p, int q) {
  if (p > q) {
    return 0;
  } else if (p == q) {
    word_t num = 0;
    sscanf(tokens[p].str, SCN_SWORD, &num);
    return num;
  } else if (check_parentheses(p, q) == true) {
    return eval(p + 1, q - 1);
  } else {
    int op = find_op(p, q);
    word_t val1 = eval(p, op - 1);
    word_t val2 = eval(op + 1, q);

    switch (tokens[op].type)
    {
    case '+': return val1 + val2;
    case '-': return val1 - val2;
    case '*': return val1 * val2;
    case '/': return val1 / val2;
    default:printf("未知的类型:%s\n", tokens[op].str);return 0;
    }
  }
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  
  word_t res = eval(0, nr_token - 1);
  *success = true;
  // /* TODO: Insert codes to evaluate the expression. */
  // TODO();

  return res;
}
