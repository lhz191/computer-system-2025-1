#include "common.h"
#include "syscall.h"
int fs_open(const char *pathname, int flags, int mode);
ssize_t fs_read(int fd, void *buf, size_t len);
ssize_t fs_write(int fd, const void *buf, size_t len);
off_t fs_lseek(int fd, off_t offset, int whence);
int fs_close(int fd);
int mm_brk(uint32_t new_brk);  // 添加mm_brk函数的声明

// // 为了强制单字符输出，sys_brk总是返回失败
// static int sys_brk(uintptr_t addr) {
//   // 返回0表示失败，这样会迫使printf()逐字符输出
//   return 0;
// }

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
    case SYS_write: {
      int fd = SYSCALL_ARG2(r);  // 获取文件描述符
      const char *buf = (const char *)SYSCALL_ARG3(r);  // 获取缓冲区地址
      size_t len = SYSCALL_ARG4(r);  // 获取写入长度
      // if (fd == 1 || fd == 2) {  // 如果是 stdout 或 stderr
      //   for (size_t i = 0; i < len; i++) {
      //     _putc(buf[i]);  // 使用 _putc 输出字符
      //   }
      //    SYSCALL_ARG1(r) = len;  // 返回写入的字节数
      // } 
      // Log("fs_write: fd=%d, buf=%p, len=%d", fd, buf, len);

      SYSCALL_ARG1(r) = fs_write(fd,buf,len);
      return NULL;
      break;
    }
    case SYS_brk: {
      uintptr_t addr = SYSCALL_ARG2(r);
      int ret = mm_brk(addr);
      SYSCALL_ARG1(r) = ret;
      break;
    }
    case SYS_open: {
      const char *pathname = (const char *)SYSCALL_ARG2(r);
      int flags = SYSCALL_ARG3(r);
      int mode = SYSCALL_ARG4(r);
      
      Log("syscall: open('%s', %d, %d)", pathname, flags, mode);
      SYSCALL_ARG1(r) = fs_open(pathname, flags, mode);
      break;
    }
    case SYS_read: {
      int fd = SYSCALL_ARG2(r);
      void *buf = (void *)SYSCALL_ARG3(r);
      size_t count = SYSCALL_ARG4(r);
      
      SYSCALL_ARG1(r) = fs_read(fd, buf, count);
      break;
    }
    case SYS_lseek: {
      int fd = SYSCALL_ARG2(r);
      off_t offset = SYSCALL_ARG3(r);
      int whence = SYSCALL_ARG4(r);
      
      Log("syscall: lseek(%d, %d, %d)", fd, offset, whence);
      SYSCALL_ARG1(r) = fs_lseek(fd, offset, whence);
      break;
    }
    case SYS_close: {
      int fd = SYSCALL_ARG2(r);
      
      Log("syscall: close(%d)", fd);
      SYSCALL_ARG1(r) = fs_close(fd);
      break;
    }
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
