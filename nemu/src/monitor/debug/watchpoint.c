#include "monitor/watchpoint.h"
#include "monitor/expr.h"
#include "monitor/monitor.h" 


#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;
/*PA1 part2 Begin*/
void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;//已使用监视点链表为空
  free_ = wp_pool;//空闲监视点链表的头指针设置为 wp_pool 的起始地址，表示所有监视点最初都是空闲的。
}
/*PA1 part2 End*/
/*PA1 part2 Begin*/
/* TODO: Implement the functionality of watchpoint */
void free_wp(WP *wp) {
  //释放监视点wp，head是已使用链表，free_是空闲链表
  WP **p = &head;
  while (*p != NULL && *p != wp) {//查找wp监视点
    p = &(*p)->next;
  }
  if (*p == NULL) return;//head为空或者wp为空，没有这个监视点，直接return

  *p = wp->next;//从head链表中（已使用链表中移除wp）
  wp->next = free_;//更新wp的next监视点
  free_ = wp;//由于wp被释放，将空闲链表头置为wp
}
/*PA1 part2 End*/
/*PA1 part2 Begin*/
void check_watchpoints() {
  WP *wp = head;
  while (wp != NULL) {
    bool success = true;
    uint32_t new_value = expr(wp->expr, &success);
    if (!success) {
      printf("Failed to evaluate expression: %s\n", wp->expr);
      return;
    }

    if (new_value != wp->last_value) {
      printf("Watchpoint %d: %s\n", wp->NO, wp->expr);
      printf("Old value = 0x%08x\n", wp->last_value);
      printf("New value = 0x%08x\n", new_value);
      wp->last_value = new_value;
      nemu_state = NEMU_STOP;
    }
    wp = wp->next;
  }
}
/*PA1 part2 End*/
/*PA1 part2 Begin*/
void print_watchpoints() {
  WP *wp = head;
  if (wp == NULL) {
    printf("No watchpoints set.\n");
  } else {
    while (wp != NULL) {
      printf("Watchpoint %d: %s\n", wp->NO, wp->expr);
      printf("Current value: 0x%08x\n", wp->last_value);
      wp = wp->next;
    }
  }
}

/*PA1 part2 End*/
/*PA1 part2 Begin*/
WP* new_wp(char *expression) { 
  if (free_ == NULL) {
    printf("No free watchpoints available.\n");
    return NULL;
  }

  // 从空闲链表中取出一个监视点
  WP *wp = free_;
  free_ = free_->next;

  // 初始化监视点
  strncpy(wp->expr, expression, sizeof(wp->expr) - 1);
  wp->expr[sizeof(wp->expr) - 1] = '\0'; // 确保字符串以 '\0' 结尾

  bool success = true;
  wp->last_value = expr(wp->expr, &success);  // 调用 expr 函数
  if (!success) {
    printf("Failed to evaluate expression: %s\n", wp->expr);
    free_wp(wp); // 如果表达式无效，释放监视点
    return NULL;
  }

  // 将监视点添加到已使用链表
  wp->next = head;
  head = wp;

  return wp;
}
/*PA1 part2 End*/
/*PA1 part2 Begin*/

WP* find_wp(int no) {
  WP *wp = head;
  while (wp != NULL) {
    if (wp->NO == no) {
      return wp;
    }
    wp = wp->next;
  }
  return NULL;
}
/*PA1 part2 End*/
