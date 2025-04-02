#include "fs.h"

typedef struct {
  char *name;
  size_t size;
  off_t disk_offset;// 文件在ramdisk中的偏移
  off_t open_offset;// 文件被打开之后的读写指针
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS, FD_DISPINFO, FD_NORMAL};

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin (note that this is not the actual stdin)", 0, 0},
  {"stdout (note that this is not the actual stdout)", 0, 0},
  {"stderr (note that this is not the actual stderr)", 0, 0},
  [FD_FB] = {"/dev/fb", 0, 0},
  [FD_EVENTS] = {"/dev/events", 0, 0},
  [FD_DISPINFO] = {"/proc/dispinfo", 128, 0},
#include "files.h"
};
extern void fb_write(const void *buf, off_t offset, size_t len);
extern void ramdisk_write(const void *buf, off_t offset, size_t len);

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

void init_fs() {
  // TODO: initialize the size of /dev/fb
}
ssize_t fs_write(int fd, const void *buf, size_t len) {
  Log("fs_write: fd=%d, buf=%p, len=%d", fd, buf, len);
  //1.stdout：将数据输出到串口或控制台。
  //2.stderr：用于标准错误输出。
  //3./dev/fb：用于写入帧缓冲区。写入的数据通常是图像数据，需要按照特定的格式（如 RGB）写入帧缓冲区。
  if (fd == FD_STDOUT || fd == FD_STDERR) {
    for (size_t i = 0; i < len; i++) {
      _putc(((char *)buf)[i]);  // 输出到串口
    }
    return len;
  }
  else if(fd == FD_FB)
  {
    fb_write(buf,file_table[fd].open_offset,len);
    file_table[fd].open_offset+=len;
    return len;
  }
  // 其他文件的写入操作
  Finfo *f = &file_table[fd];
  if (f->open_offset + len > f->size) {
    len = f->size - f->open_offset;  // 调整写入长度
  }
  ramdisk_write(buf,f->disk_offset + f->open_offset, len);
  f->open_offset += len;  // 更新偏移量
  return len;
}