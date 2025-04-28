#include "common.h"
_RegSet* do_syscall(_RegSet *r);
static _RegSet* do_event(_Event e, _RegSet* r) {
  switch (e.event) {
    case _EVENT_SYSCALL:
      return do_syscall(r);
    case _EVENT_TRAP:
      Log("BAD TRAP: cause=%d, eip=0x%08x", e.cause, r->eip);
      Log("Register state: eax=0x%08x, ebx=0x%08x, ecx=0x%08x, edx=0x%08x", 
          r->eax, r->ebx, r->ecx, r->edx);
      Log("esp=0x%08x, ebp=0x%08x, esi=0x%08x, edi=0x%08x", 
          r->esp, r->ebp, r->esi, r->edi);
      panic("BAD TRAP");
      break;
    default: panic("Unhandled event ID = %d", e.event);
  }
  return NULL;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  _asye_init(do_event);
}