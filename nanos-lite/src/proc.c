#include "../include/proc.h"
#include "../include/fs.h"

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC];
static int nr_proc = 0;
PCB *current = NULL;

// 用于优先级调度的计数器
static int count = 0;
// 每PAL_PRIORITY次调度中，让hello程序只运行1次
#define PAL_PRIORITY 5

// 当前游戏：0表示仙剑奇侠传，2表示videotest
static int current_game = 0;

// 在仙剑奇侠传和videotest之间切换
void switch_game() {
  current_game = (current_game == 0) ? 2 : 0;
  printf("Switching to %s\n", current_game == 0 ? "Pal" : "VideoTest");
}

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
  
  // 优先级调度：在当前游戏和hello程序之间切换
  count = (count + 1) % PAL_PRIORITY;
  
  if (count == 0 && nr_proc > 1) {
    // 每PAL_PRIORITY次调度中，选择一次hello程序(index 1)
    current = &pcb[1];
  } else {
    // 其他时间选择当前游戏(index 0或2)
    current = &pcb[current_game];
  }
  
  // 切换到新进程的地址空间
  _switch(&current->as);
  
  // 返回新进程的陷阱帧
  return current->tf;
}
