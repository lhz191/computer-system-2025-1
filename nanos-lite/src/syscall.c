#include "common.h"
#include "syscall.h"

_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4]; // 用于存储系统调用参数
  a[0] = SYSCALL_ARG1(r);// 获取系统调用号，根据之前的实现，存储在eax中

  switch (a[0]) {
    case SYS_none:{
    //SYS_none 是一个系统调用的标识符
    //通常用于表示一个不执行任何操作的系统调用。返回值置1，return null
      SYSCALL_ARG1(r) = 1;
      break;
    }
    case SYS_exit: {
      // 获取退出状态参数
      uintptr_t exit_status = SYSCALL_ARG2(r);
      _halt(exit_status);  // 调用 _halt() 以退出
      break;
    }
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
