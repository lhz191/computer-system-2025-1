#ifndef __PROC_H__
#define __PROC_H__

#include "common.h"
#include "memory.h"

#define STACK_SIZE (8 * PGSIZE)

typedef union {
  uint8_t stack[STACK_SIZE] PG_ALIGN;
  struct {
    _RegSet *tf;
    _Protect as;
    uintptr_t cur_brk;
    // we do not free memory, so use `max_brk' to determine when to call _map()
    uintptr_t max_brk;
  };
} PCB;

extern PCB *current;

// 添加loader函数的声明
uintptr_t loader(_Protect *as, const char *filename);

// 添加schedule函数的声明
_RegSet* schedule(_RegSet *prev);

#endif
