#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"
/*PA1 part2 Begin*/
typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  char expr[256]; // 存储表达式
  uint32_t last_value; // 上次计算的值
  /* TODO: Add more members if necessary */
} WP;

void init_wp_pool();
WP* new_wp(char *expression);
void free_wp(WP *wp);
void check_watchpoints();
void print_watchpoints(); 
WP* find_wp(int no);
#endif
/*PA1 part2 End*/