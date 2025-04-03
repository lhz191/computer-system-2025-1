#include "common.h"

#define DEFAULT_ENTRY ((void *)0x4000000)

size_t get_ramdisk_size();
void ramdisk_read(void *buf, off_t offset, size_t len);

uintptr_t loader(_Protect *as, const char *filename) {
  // size_t size = get_ramdisk_size();
  // ramdisk_read(DEFAULT_ENTRY, 0, size);
  // return (uintptr_t)DEFAULT_ENTRY;


  int fd = fs_open(filename, 0, 0);// 打开指定的文件
  if (fd < 0) {
    panic("loader: cannot open file '%s'", filename);
  }
  size_t size = fs_filesz(fd);// 获取文件大小
  Log("loader: loading '%s' (%d bytes) to memory address 0x%x", 
      filename, size, (uintptr_t)DEFAULT_ENTRY);
  fs_read(fd, DEFAULT_ENTRY, size);
  fs_close(fd);
  return (uintptr_t)DEFAULT_ENTRY;//返回程序入口地址  
}
