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

#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

word_t vaddr_read(vaddr_t addr, int len);

enum {
  TK_NOTYPE = 256, TK_EQ, TK_DEC, TK_HEX, TK_NEQ, TK_AND, TK_REG, TK_DEF,

  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"\\-", '-'},         // 减
  {"\\*", '*'},         // 乘
  {"\\/", '/'},         // 除
  {"\\(", '('},         // 左括号
  {"\\)", ')'},         // 右括号
  {"==", TK_EQ},        // equal
  {"!=", TK_NEQ},       // 不等
  {"&&", TK_AND},       // 逻辑与
  {"0x[1-9A-Fa-f][0-9A-Fa-f]*", TK_HEX},//十六进制整数
  {"[1-9][0-9]*", TK_DEC}, //十进制整数
  {"\\$(pc|\\$0|ra|sp|gp|tp|t0|t1|t2|s0|s1|a0|a1|a2|a3|a4|a5|a6|a7|s2|s3|s4|s5|s6|s7|s8|s9|s10|s11|t3|t4|t5|t6)", TK_REG},//寄存器

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

static Token tokens[256] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool check_parentheses(int p, int q, bool *legal)
{
  int i = 0;
  if(tokens[p].type == '(' && tokens[q].type == ')')
  {
    for(int j = p + 1; j < q ; j++)
    {
      if(tokens[j].type == '(')
      {
        i = i + 1;
      }

      if(tokens[j].type == ')')
      {
        i = i - 1;
      }

      if(i < 0)
      {
        break;
      }
    }

    if(i == 0)
    {
      *legal = true;
      return true;
    }
    if(i > 0)
    {
      *legal = false;
      return false;
    }

  }
  i = 0;
  
  for(int k = p; k <= q ; k++)
  {
    if(tokens[k].type == '(')
    {
      i = i + 1;
    }

    if(tokens[k].type == ')')
    {
      i = i - 1;
    }

    if(i < 0)
    {
      *legal = false;
      return false;
    }
  }

  if(i == 0)
  {
    *legal = true;
    return false;
  }
  else
  {
    *legal = false;
    return false;
  }
  
}

static int prior_op(int op)
{
  switch (op)
  {
    case TK_AND:return 1;
    case TK_EQ: return 2;
    case TK_NEQ:return 2;
    case '+':   return 3;
    case '-':   return 3;
    case '*':   return 4;
    case '/':   return 4;
    case TK_DEF:return 5;
  default: return 6;
  }
}

static int find_op(int p, int q)
{
  int op = p;
  int op_prior = 6;
  int cnt = 0;
  for(int i = p; i <= q; i++)
  {
    if(tokens[i].type == '(')
    {
      cnt = cnt + 1;
      continue;
    }
    if(tokens[i].type == ')')
    {
      cnt = cnt - 1;
      continue;
    }
    if((prior_op(tokens[i].type) <= op_prior) && cnt == 0)
    {
      op = i;
      op_prior = prior_op(tokens[i].type);
    }
  }

  return op;
}

static uint32_t eval(int p, int q)
{
  bool rabbish;
  if(p > q)
  {
    /* Bad expression */
    Assert(0, "表达式求值错误");
    return 0;
  }
  else if(p == q)
  {
    /* Single token.
     * For now this token should be a number.
     * Return the value of the number.
     */
    uint32_t value;
    if(tokens[p].type == TK_DEC)
    {
      sscanf(tokens[p].str, "%u", &value);
      return value;
    }
    
    if(tokens[p].type == TK_HEX)
    {
      sscanf(tokens[p].str, "%x", &value);
      return value;
    }

    if(tokens[p].type == TK_REG)
    {
      bool success = false;
      value = isa_reg_str2val(tokens[p].str, &success);
      if(success == false)
      {
        Assert(0, "寄存器输入错误");
      }
      else
      {
        return value;
      }
    }
    Assert(0, "求值错误");
  }
  else if(check_parentheses(p, q, &rabbish) == true)
  {
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses.
     */
    return eval(p + 1, q - 1);
  }
  else
  {
    int op = find_op(p, q);
    uint32_t val1 = 0;
    if(tokens[op].type != TK_DEF)
      val1 = eval(p, op - 1);
    uint32_t val2 = eval(op + 1, q);

    switch (tokens[op].type)
    {
    case '+': return val1 + val2;
    case '-': return val1 - val2;
    case '*': return val1 * val2;
    case '/': return val1 / val2;
    case TK_EQ:return (val1 == val2);
    case TK_NEQ:return (val1 != val2);
    case TK_AND:return (val1 && val2);
    case TK_DEF:return vaddr_read(val2, 4);
    default: assert(0);
    }
  }

}
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

        if(rules[i].token_type == TK_NOTYPE)
        {
          break;//如果token是空格，跳过该token
        }
        
        tokens[nr_token].type = rules[i].token_type;//先将类型写入

        switch (rules[i].token_type) {
          case TK_DEC:
                    strncpy(tokens[nr_token].str, substr_start, substr_len);
                    tokens[nr_token].str[substr_len] = '\0';
                    break;
          case TK_HEX:
                    strncpy(tokens[nr_token].str, substr_start, substr_len);
                    tokens[nr_token].str[substr_len] = '\0';
                    break;
          case TK_REG:
                    strncpy(tokens[nr_token].str, substr_start + 1, substr_len - 1);
                    tokens[nr_token].str[substr_len - 1] = '\0';
        }
        nr_token++;
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}


word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  
  for(int i = 0; i < nr_token; i++)
  {
    if(tokens[i].type == '*')
    {
      if(i == 0)
      {
        tokens[i].type = TK_DEF;
      }
      else if(tokens[i-1].type == TK_DEC || tokens[i-1].type == TK_HEX || tokens[i-1].type == ')')
      {
        tokens[i].type = '*';
      }
      else
      {
        tokens[i].type = TK_DEF;
      }
    }
    
  }

  bool legal = false;
  check_parentheses(0, nr_token - 1, &legal);
  if (legal == true)
  {
    printf("表达式合法\n");
    *success = true;
    return eval(0, nr_token - 1);
  }
  else
  {
    printf("表达式不合法\n");
    *success = false;
    return 0;
  }

  
}
