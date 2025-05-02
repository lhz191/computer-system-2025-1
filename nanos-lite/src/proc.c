#include "../include/proc.h"
#include "../include/fs.h"

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC];
static int nr_proc = 0;
PCB *current = NULL;


void load_prog(const char *filename) {
  int i = nr_proc ++;
  _protect(&pcb[i].as);

  uintptr_t entry = loader(&pcb[i].as, filename);

  // Comment out the following three lines after implementing _umake()
  // _switch(&pcb[i].as);
  // current = &pcb[i];
  // ((void (*)(void))entry)();

  _Area stack;
  stack.start = pcb[i].stack;
  stack.end = stack.start + sizeof(pcb[i].stack);

  pcb[i].tf = _umake(&pcb[i].as, stack, stack, (void *)entry, NULL, NULL);
}

_RegSet* schedule(_RegSet *prev) {
  // Save the context of the current process
  if (current != NULL) {
    current->tf = prev;
  }
  
  // 简单的轮流调度：在仙剑奇侠传和hello程序之间切换
  current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);
  
  // 切换到新进程的地址空间
  _switch(&current->as);
  
  // 返回新进程的陷阱帧
  return current->tf;
}
