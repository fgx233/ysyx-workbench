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
#include <stdarg.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_DEC,

  /* TODO: Add more token types */

};

enum OP_CLASS {
  ONE, TWO, THREE, FOUR, FIVE
};

enum ERR_TYPE {
  OVER, NUM, BRACKET, OP, DIV, UNKNOWN
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

void print_err(int type, int error_num, ...) {

  printf("读取字符串:");
  for (int i = 0; i < nr_token; i++)
  {
    printf("%s", tokens[i].str);
  }
  printf("\n");

  va_list ptr;

  va_start(ptr, error_num);

  
  int position1 = 0;
  int position2 = 0;

  int p1_len = 0;
  int p2_len = 0;

  if (error_num == 1) {
    position1 = va_arg(ptr, int);
    for (int i = 0; i < position1; i++) {
      p1_len += strlen(tokens[i].str);
    }
  } else if (error_num == 2) {
    position1 = va_arg(ptr, int);
    position2 = va_arg(ptr, int);
    for (int i = 0; i < position1; i++) {
      p1_len += strlen(tokens[i].str);
    }
    for (int i = 0; i < position2; i++) {
      p2_len += strlen(tokens[i].str);
    }
  } else {
    printf("错误打印函数接收了错误的参数\n");
    return;
  }

  printf("错误的地方:");
  
  switch (type)
  {
  case OVER:
            printf("%*s%s\n", p2_len, "", "^^");
            printf("p<q, 求值越界/空括号\n");
            break;
  case NUM:
            printf("%*s%c\n", p1_len, "", '^');
            printf("p=q, 此token非数字类型\n");
            break;
  case BRACKET:
            printf("%*s%c", p1_len, "", '^');
            printf("%*s%c\n", p2_len - p1_len - 1, "", '^');
            printf("check_parentheses, 括号数量不匹配\n");
            break;
  case OP:
            printf("%*s%c", p1_len, "", '^');
            printf("%*s%c\n", p2_len - p1_len - 1, "", '^');
            printf("find_op, 寻找主运算符失败\n");
            break;
  case DIV:
            printf("%*s%c\n", p1_len, "", '^');
            printf("div, 发生除0行为\n");
            break;
  case UNKNOWN:
            printf("%*s%c\n", p1_len, "", '^');
            printf("此运算符类型未知\n");
            break;
  default:  printf("错误打印函数接收了错误的参数\n");
            return;
  }
  return;
}

bool check_parentheses(int p, int q, bool *success) {
    int nr_brace = 0;
    bool flag = true;

    if (tokens[p].type == '(' && tokens[q].type == ')') {
        for (int i = p; i <= q; i ++) {
            if (tokens[i].type == '(') {
                nr_brace += 1;
            }
            if (tokens[i].type == ')') {
                nr_brace -= 1;
            }
            if (nr_brace < 0 || (nr_brace == 0 && i != q)) {
                flag = false;
            }
        }
        
        if (nr_brace == 0 && flag == true) {
            return true;
        } else if (nr_brace == 0 && flag == false) {
            return false;
        }
        print_err(BRACKET, 2, p, q);
        *success = false;
        return false;
    }

    return false;
}

int find_op(int p, int q) {
  int op = p;
  int old_class = FIVE;
  int new_class = ONE;
  int jump = 0;
  for(int i = p; i <= q; i++) {

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
    case '+': case '-': new_class = ONE;break;
    case '*': case '/': new_class = TWO;break;
    default: new_class = FIVE;
    }

    if (old_class >= new_class) {
      old_class = new_class;
      op = i;
    }
  }

  if (old_class == FIVE) {
    return -1;
  }
  return op;
}

word_t eval(int p, int q, bool *success) {

  if (*success == false) {       //若计算中出错，跳过剩余计算
    return 0;
  }

  if (p > q) {
    print_err(OVER, 2, p, q);
    *success = false; 
    return 0;
  } else if (p == q) {
    if (tokens[p].type != TK_DEC) {
      print_err(NUM, 1, p);
      *success = false;
      return 0;
    }
    word_t num = 0;
    sscanf(tokens[p].str, SCN_SWORD, &num);
    return num;
  } else if (check_parentheses(p, q, success) == true) {
    return eval(p + 1, q - 1, success);
  } else {
    if(*success == false) {
      return 0;
    }

    int op = find_op(p, q);
    if (op == -1) {
      *success = false;
      print_err(OP, 2, p, q);
      return 0;    
    }
    word_t val1 = eval(p, op - 1, success);
    word_t val2 = eval(op + 1, q, success);

    if (*success == false) {
      return 0;
    }

    switch (tokens[op].type)
    {
    case '+': return val1 + val2;
    case '-': return val1 - val2;
    case '*': return val1 * val2;
    case '/': if (val2 == 0) {
                *success = false;
                print_err(DIV, 1, op + 1);
                for (int i = op + 1; i <= q; i ++) {
                  printf("%s",tokens[i].str);
                }
                printf("\n");
                return 0;
              }
              return val1 / val2;
    default: *success = false; print_err(UNKNOWN, 1, op);return 0;
    }
  }
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  
  word_t res = eval(0, nr_token - 1, success);

  return res;
}
