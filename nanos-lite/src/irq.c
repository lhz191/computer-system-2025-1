#include "common.h"
#include "../include/proc.h"

_RegSet* do_syscall(_RegSet *r);
static _RegSet* do_event(_Event e, _RegSet* r) {
  switch (e.event) {
    case _EVENT_SYSCALL:
      // 处理完系统调用后进行进程调度
      do_syscall(r);
      return schedule(r);
    case _EVENT_TRAP:
      
      printf("Received kernel trap, initiating context switch.\n");
      // Schedule will switch to the first user process
      return schedule(r);
    case _EVENT_IRQ_TIME:
      // Pa4.3 处理时钟中断，进行进程调度
      printf("Timer interrupt, initiating context switch.\n");
      return schedule(r);
    default: panic("Unhandled event ID = %d", e.event);
  }
  return NULL;
}

void init_irq(void) {
  _asye_init(do_event);
}
