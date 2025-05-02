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
    int fd=fs_open(filename,0,0);
    // Log("fd=%d",fd);
    // fs_read(fd,DEFAULT_ENTRY,fs_filesz(fd));
    int size=fs_filesz(fd);
    int pnums=size/PGSIZE;
    if(size%PGSIZE==0)  pnums++;
    void *pa=NULL;
    void *va=DEFAULT_ENTRY;
    for(int i=0;i<=pnums;i++)
    {
        pa=new_page();//申请空闲页
        _map(as,va,pa);//物理页->用户程序虚拟地址
        fs_read(fd,pa,PGSIZE);//读一页
        va+=PGSIZE;
        // Log("%x",va);
    }
    fs_close(fd);
  return (uintptr_t)DEFAULT_ENTRY;
}