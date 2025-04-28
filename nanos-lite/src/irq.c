#include "common.h"
_RegSet* do_syscall(_RegSet *r);
static _RegSet* do_event(_Event e, _RegSet* r) {
  switch (e.event) {
    case _EVENT_SYSCALL:
      return do_syscall(r);
    case _EVENT_TRAP:
     printf("BAD TRAP: %d at eip = 0x%x\n", e.cause, r->eip);
     // 打印寄存器状态
     printf("eax = 0x%x, ebx = 0x%x, ecx = 0x%x, edx = 0x%x\n", 
            r->eax, r->ebx, r->ecx, r->edx);
     // ...然后再panic
     panic("BAD TRAP");
      break;
    default: panic("Unhandled event ID = %d", e.event);
  }
  return NULL;
}

void init_irq(void) {
  _asye_init(do_event);
}
