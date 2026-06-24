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

#include "sdb.h"

#define NR_WP 32



static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp(char *e) {
  if (free_ == NULL) {
    printf("当前监视点已满，无法添加新的监视点。\n");
    return NULL;
  }

  WP* p = free_;
  free_ = free_->next;
  p->next = head;
  head = p;

  int n = snprintf(p->expr, sizeof(p->expr), "%s", e);
  if (n >= sizeof(p->expr)) {
    printf("表达式过长，监视点添加失败。\n");
    free_wp(p);
    return NULL;
  }

  bool success = true;
  p->old_val = expr(e, &success);
  if (success == false) {
    printf("表达式求值失败");
    free_wp(p);
    return NULL;
  }
  return p;
}

void free_wp(WP *wp) {
  if (wp == NULL || head == NULL) {
    printf("删除监视点错误\n");
    return;
  }

  WP* pre = head;
  WP* cur = head;
  while (cur != NULL) {
    if (cur == wp) {
      if (cur == head) {
        head = head->next;
        cur->next = free_;
        free_ = cur;
        return;
      } else {
        pre->next = cur->next;
        cur->next = free_;
        free_ = cur;
        return;
      }
    } else {
      pre = cur;
      cur = cur->next;
    }
  }
  
}

void print_all() {
  WP *p = head;
  
  int max = 12;
  while (p != NULL) {
    if (max < strlen(p->expr)) {
      max = strlen(p->expr);
    }
    p = p->next;
  }
  printf("%-8s%-*s%-11s%-11s\n", "NO", max+1, "Expr", "Value(Hex)", "Value(Dec)");
  p = head;
  while (p != NULL) {
    printf("%-8d%-*s" FMT_WORD " " FMT_SWORD "\n", p->NO, max+1, p->expr, p->old_val, (sword_t)p->old_val);
    p = p->next;
  }
}

void delete_wp(int NO) {
  WP *p = head;
  while (p != NULL) {
    if (p->NO == NO) {
      free_wp(p);
      return;
    } else {
      p = p->next;
    }
  }
  printf("没有此监视点:No=%d\n", NO);

}

void check_wp() {
  if (head == NULL) {
    return;
  }

  WP* p = head;

  while (p != NULL) {
    bool success = true;
    word_t new = expr(p->expr, &success);
    if (success == false) {
      printf("监视点求值出错:NO=%d, Expr=%s\n", p->NO, p->expr);
      if (nemu_state.state == NEMU_RUNNING) {
        set_nemu_state(NEMU_STOP, cpu.pc, 0);
      }
      
      return;
    }

    p->new_val = new;
    p = p->next;
  }

  p = head;
  bool flag = false;

  while (p != NULL) {
    if (p->old_val != p->new_val) {
      printf("监视点触发：NO=%d  expr = %s, new = " FMT_WORD "(" FMT_SWORD ")" " old = " FMT_WORD "(" FMT_SWORD ")" "\n", p->NO, p->expr, p->new_val, (sword_t)p->new_val, p->old_val, (sword_t)p->old_val);
      p->old_val = p->new_val;
      flag = true;
    }
    p = p->next;
  }

  if (flag == true && nemu_state.state == NEMU_RUNNING) {
    set_nemu_state(NEMU_STOP, cpu.pc, 0);
  }
}