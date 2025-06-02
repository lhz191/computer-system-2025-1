#include "common.h"

/* Uncomment these macros to enable corresponding functionality. */
#define HAS_ASYE
#define HAS_PTE

void init_mm(void);
void init_ramdisk(void);
void init_device(void);
void init_irq(void);
void init_fs(void);
void load_prog(const char *filename);
uint32_t loader(_Protect *, const char *);

int main() {
#ifdef HAS_PTE
  init_mm();
#endif

  Log("'Hello World!' from Nanos-lite");
  Log("Build time: %s, %s", __TIME__, __DATE__);

  init_ramdisk();

  init_device();

#ifdef HAS_ASYE
  Log("Initializing interrupt/exception handler...");
  init_irq();
#endif

  init_fs();

  // uint32_t entry = loader(NULL, "/bin/text");
  // /bin/bmptest
  // uint32_t entry = loader(NULL, "/bin/bmptest");
  // /bin/events
  // uint32_t entry = loader(NULL, "/bin/events");
//   // /bin/pal
// uint32_t entry = loader(NULL, "/bin/pal");
//   ((void (*)(void))entry)();
load_prog("/bin/pal");  // Pa4.1: 使用load_prog加载用户程序到独立的虚拟地址空间
load_prog("/bin/hello");
// load_prog("/bin/videotest");

_trap();  // Use kernel self-trap to switch to user process

  panic("Should not reach here");
}
