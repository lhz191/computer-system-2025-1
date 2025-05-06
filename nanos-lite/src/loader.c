#include "common.h"
#include "fs.h"
#include "memory.h"

#define DEFAULT_ENTRY ((void *)0x8048000)  // Pa4.1: 修改程序入口地址为0x8048000

size_t get_ramdisk_size();
void ramdisk_read(void *buf, off_t offset, size_t len);
void* new_page(void);

// Pa4.1: 增加文件系统函数声明
int fs_open(const char *pathname, int flags, int mode);
size_t fs_filesz(int fd);
ssize_t fs_read(int fd, void *buf, size_t len);
int fs_close(int fd);

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
      filename, size, DEFAULT_ENTRY);
  
  // Pa4.1: 以页为单位加载用户程序，使其运行在独立的虚拟地址空间
  int pages = (size + PGSIZE - 1) / PGSIZE; // 向上取整，计算需要多少页
  void *pa;
  void *va = (void *)DEFAULT_ENTRY;
  
  for (int i = 0; i < pages; i++) {
    // 1. 申请一页空闲的物理页
    pa = new_page();
    
    // 2. 把这一物理页映射到用户程序的虚拟地址空间中
    _map(as, va, pa);  // Pa4.1: 修正_map参数，去掉不需要的权限参数
    
    // 3. 从文件中读入一页或剩余内容到这一物理页上
    size_t bytes_to_read = (i == pages - 1) ? (size - i * PGSIZE) : PGSIZE;
    fs_read(fd, pa, bytes_to_read);
    
    // 移动虚拟地址指针到下一页
    va += PGSIZE;
  }
  
  fs_close(fd);
  return (uintptr_t)DEFAULT_ENTRY;//返回程序入口地址  
}