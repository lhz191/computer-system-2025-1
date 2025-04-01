#ifndef __ARCH_H__
#define __ARCH_H__

#include <am.h>

#define PMEM_SIZE (128 * 1024 * 1024)
#define PGSIZE    4096    // Bytes mapped by a page

struct _RegSet {
  uintptr_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
  int       irq;
  uintptr_t error_code;
  uintptr_t eip, cs, eflags;
};

#define SYSCALL_ARG1(r) ((r)->eax)  // 系统调用号
#define SYSCALL_ARG2(r) ((r)->ebx)  // 第一个参数
#define SYSCALL_ARG3(r) ((r)->ecx)  // 第二个参数
#define SYSCALL_ARG4(r) ((r)->edx)  // 第三个参数

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif
#endif
