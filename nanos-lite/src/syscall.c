#include "common.h"
#include "syscall.h"
// int fs_open(const char *pathname, int flags, int mode);
// ssize_t fs_read(int fd, void *buf, size_t len);
ssize_t fs_write(int fd, const void *buf, size_t len);
// off_t fs_lseek(int fd, off_t offset, int whence);
// int fs_close(int fd);
static inline _RegSet* sys_write(_RegSet *r){
  /*int fd = (int)SYSCALL_ARG2(r);
  char *buf = (char *)SYSCALL_ARG3(r);
  int len = (int)SYSCALL_ARG4(r);
  //Log("?");
  if(fd == 1 || fd == 2){
      for(int i = 0; i < len; i++) {
          _putc(buf[i]);
      }
      //根据man 返回len
      SYSCALL_ARG1(r) = SYSCALL_ARG4(r);
  }
  return NULL;*/
  int fd = (int)SYSCALL_ARG2(r);
  char *buf = (char *)SYSCALL_ARG3(r);
  int len = (int)SYSCALL_ARG4(r);
  SYSCALL_ARG1(r) = fs_write(fd,buf,len);
  return NULL;
}

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
return sys_write(r);
    }
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
