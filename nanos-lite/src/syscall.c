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
    // case SYS_write: {
    //   int fd = SYSCALL_ARG2(r);  // 获取文件描述符
    //   const char *buf = (const char *)SYSCALL_ARG3(r);  // 获取缓冲区地址
    //   size_t len = SYSCALL_ARG4(r);  // 获取写入长度

    //   if (fd == 1 || fd == 2) {  // 如果是 stdout 或 stderr
    //     for (size_t i = 0; i < len; i++) {
    //       _putc(buf[i]);  // 使用 _putc 输出字符
    //     }
    //     // SYSCALL_ARG1(r) = len;  // 返回写入的字节数
    //   } 
    //   break;
    // }
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
