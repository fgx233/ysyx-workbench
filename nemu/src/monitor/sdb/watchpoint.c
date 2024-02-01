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

#include "sdb.h"

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  char expr[256];
  uint32_t old_val;
  /* TODO: Add more members if necessary */

} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void new_wp(char *args)
{
  bool success = false;
  uint32_t result = expr(args, &success);
  
  if(success == false)
  {
    printf("添加监视点失败，表达式不合法\n");
    return;
  }

  if(free_ == NULL)
  {
    printf("监视点已全部用光\n");
    return;
  }

  if(head == NULL)
  {
    head = free_;
    free_ = free_->next;
    head->next = NULL;
    // sscanf(head->expr, "%[^\n]", args);
    strcpy(head->expr, args);
    head->old_val = result;
    printf("添加监视点%d：%s\n",head->NO, head->expr);
    return;
  }
  else
  {
    WP *temp = free_;
    free_ = free_->next;
    temp->next = head;
    head = temp;
    strcpy(head->expr, args);
    head->old_val = result;
    printf("添加监视点%d：%s\n",head->NO, head->expr);
    return;
  }
}

void free_wp(int NO)
{
  WP *temp = head;

  if(head == NULL)
  {
    printf("当前监视点为空\n");
    return;
  }

  if(head->NO == NO)
  {
    head->next = free_;
    free_ = head;
    head = NULL;
    printf("删除成功\n");
    return;
  }
  while(temp->next != NULL)
  {
    if(temp->next->NO == NO)
    {
      WP *tmp = temp->next;
      temp->next = temp->next->next;
      tmp->next = free_;
      free_ = tmp;
      printf("删除成功\n");
      return;
    }
    temp = temp->next;
  }
  printf("删除失败\n");
  return;
}

void print_wp()
{
  WP *temp = head;
  printf("Num     What\n");
  while (temp != NULL)
  {
    printf("%-8d%s\n",temp->NO, temp->expr);
    temp = temp->next;
  }
  return;
}

void update_wp()
{
  WP *temp = head;
  while (temp != NULL)
  {
    bool success = false;
    uint32_t new_val = expr(temp->expr, &success);
    if(new_val != temp->old_val)
    {
      printf("Hardware watchpoint %d: %s\n", temp->NO, temp->expr);
      printf("Old value = %u\n", temp->old_val);
      printf("New value = %u\n", new_val);
      temp->old_val = new_val;
      nemu_state.state = NEMU_STOP;
    }
    temp = temp->next;
  }
  return;
}

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
    wp_pool[i].old_val = 0;
    wp_pool[i].expr[0] = '\0';
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

