#ifndef __SDB_H__
#define __SDB_H__

#include "utils.hpp"

// sdb.c
void sdb_set_batch_mode();
void sdb_mainloop();
void init_sdb();
// expr.c
void init_regex();
word_t expr(char *e, bool *success);
// watchpoint.c
void init_wp_pool();


// watchpoint
typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  char expr[256];
  word_t old_val;
  word_t new_val;
} WP;

WP* new_wp(char *e);
void delete_wp(int NO);
void free_wp(WP *wp);
void print_all();
#endif