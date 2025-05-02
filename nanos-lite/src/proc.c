#include "proc.h"

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
  
  // Always select the first process (only one user process for now)
  if (nr_proc > 0) {
    current = &pcb[0];
    _switch(&current->as);
    return current->tf;
  }
  
  // No processes available
  return NULL;
}
