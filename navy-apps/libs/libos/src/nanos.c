#include <unistd.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <assert.h>
#include <time.h>
#include "syscall.h"

// TODO: discuss with syscall interface
#ifndef __ISA_NATIVE__

// FIXME: this is temporary

int _syscall_(int type, uintptr_t a0, uintptr_t a1, uintptr_t a2){
  int ret = -1;
  asm volatile("int $0x80": "=a"(ret): "a"(type), "b"(a0), "c"(a1), "d"(a2));
  return ret;
}

void _exit(int status) {
  _syscall_(SYS_exit, status, 0, 0);
}

int _open(const char *path, int flags, mode_t mode) {
  _exit(SYS_open);
}
/*Pa3.2 write系统调用*/
int _write(int fd, void *buf, size_t count){
  return _syscall_(SYS_write, fd, (uintptr_t)buf, count);
}

// 声明_end符号，它标记数据段结束的位置
extern char _end;
// 用于记录当前program break的位置
static uintptr_t program_break = 0;

void *_sbrk(intptr_t increment) {
  // 第一次调用时初始化program break
  if (program_break == 0) {
    program_break = (uintptr_t)&_end;
  }
  // 保存旧的program break
  uintptr_t old_break = program_break;
  // 计算新的program break
  uintptr_t new_break = program_break + increment;
  // 调用SYS_brk系统调用设置新的program break
  int ret = _syscall_(SYS_brk, new_break, 0, 0);
  Log("[DEBUG] _sbrk called: increment=%d, old_break=0x%lx, new_break=0x%lx, ret=%d", 
      increment, old_break, new_break, ret);
  if (ret == 0) {
    // 系统调用成功，更新记录的program break
    program_break = new_break;
    // 返回旧的program break位置
    return (void *)old_break;
  } else {
    // 系统调用失败，返回-1
    return (void *)-1;
  }
}

int _read(int fd, void *buf, size_t count) {
  _exit(SYS_read);
}

int _close(int fd) {
  _exit(SYS_close);
}

off_t _lseek(int fd, off_t offset, int whence) {
  _exit(SYS_lseek);
}

// The code below is not used by Nanos-lite.
// But to pass linking, they are defined as dummy functions

// not implement but used
int _fstat(int fd, struct stat *buf) {
  return 0;
}

int execve(const char *fname, char * const argv[], char *const envp[]) {
  assert(0);
  return -1;
}

int _execve(const char *fname, char * const argv[], char *const envp[]) {
  return execve(fname, argv, envp);
}

int _kill(int pid, int sig) {
  _exit(-SYS_kill);
  return -1;
}

pid_t _getpid() {
  _exit(-SYS_getpid);
  return 1;
}

char **environ;

time_t time(time_t *tloc) {
  assert(0);
  return 0;
}

int signal(int num, void *handler) {
  assert(0);
  return -1;
}

pid_t _fork() {
  assert(0);
  return -1;
}

int _link(const char *d, const char *n) {
  assert(0);
  return -1;
}

int _unlink(const char *n) {
  assert(0);
  return -1;
}

pid_t _wait(int *status) {
  assert(0);
  return -1;
}

clock_t _times(void *buf) {
  assert(0);
  return 0;
}

int _gettimeofday(struct timeval *tv) {
  assert(0);
  tv->tv_sec = 0;
  tv->tv_usec = 0;
  return 0;
}

int _fcntl(int fd, int cmd, ... ) {
  assert(0);
  return 0;
}

int pipe(int pipefd[2]) {
  assert(0);
  return 0;
}

int dup(int oldfd) {
  assert(0);
  return 0;
}

int dup2(int oldfd, int newfd) {
  assert(0);
  return 0;
}

pid_t vfork(void) {
  assert(0);
  return 0;
}

#endif
