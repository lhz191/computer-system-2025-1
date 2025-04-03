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
extern void ramdisk_read(void *buf, off_t offset, size_t len);
extern void ramdisk_write(const void *buf, off_t offset, size_t len);
extern void fb_write(const void *buf, off_t offset, size_t len);
extern void dispinfo_read(void *buf, off_t offset, size_t len);
extern size_t events_read(void *buf, size_t len);

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

void init_fs() {
  // TODO: initialize the size of /dev/fb
  file_table[FD_FB].size = _screen.height * _screen.width * 4;
  Log("File system initialized, framebuffer size: %d x %d (%d bytes)", 
      _screen.width, _screen.height, file_table[FD_FB].size);
}

/*Pa3.2 fs_open*/
// 打开文件
int fs_open(const char *pathname, int flags, int mode) {
  for (int i = 0; i < sizeof(file_table) / sizeof(Finfo); i++) {
    if (strcmp(file_table[i].name, pathname) == 0) {
      Log("find!!");
      return i;  // 返回文件描述符（即文件记录表索引）
    }
  }
  panic("fs_open: file '%s' not found", pathname);// 未找到文件
  assert(0);
  return -1;
}


//Pa3.2 获取文件大小
size_t fs_filesz(int fd) {
  // assert(fd >= 0 && fd < sizeof(file_table) / sizeof(Finfo));
  return file_table[fd].size;
}

/*Pa3.2 读取文件*/
ssize_t fs_read(int fd, void* buf, size_t len) {
  // assert(fd >= 0 && fd < NR_FILES);
  ssize_t file_size = fs_filesz(fd);
  size_t current_offset = file_table[fd].open_offset;
  if (current_offset + len > file_size) {
    len = file_size - current_offset;
  }
  switch (fd) {
    case FD_STDIN:
    case FD_STDOUT:
    case FD_STDERR:
      // 标准IO无需实际读取内容
      return 0;
    case FD_EVENTS:
      // 从事件源读取数据（如键盘输入）
      len = events_read(buf, len);
      break;
    case FD_DISPINFO:
      // 读取显示设备信息
      dispinfo_read(buf, current_offset, len);
      file_table[fd].open_offset += len;
      break;
    default:
      // 普通文件从ramdisk读取
      if (len > 0) {
        off_t disk_pos = file_table[fd].disk_offset + current_offset;
        ramdisk_read(buf, disk_pos, len);
        file_table[fd].open_offset += len;
      }
      break;
  }
  return len;
}


//Pa3.2 写入文件
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


off_t fs_lseek(int fd, off_t offset, int whence) {
  // assert(fd >= 0 && fd < NR_FILES);
  Finfo *file = &file_table[fd];
  off_t new_offset = -1;
  switch (whence) {
    case SEEK_SET: // 从文件开头计算
    if(offset>=0 && offset<=fs_filesz(fd)){
      file->open_offset = offset;
      new_offset = offset;
    }
    break;
    case SEEK_CUR: // 从当前位置计算
    if(offset+file->open_offset>=0 && offset+file->open_offset<=fs_filesz(fd)){
      file->open_offset += offset;
      new_offset = offset;
    }
    break;
    case SEEK_END: // 从文件末尾计算
      file->open_offset = fs_filesz(fd) + offset;
      new_offset = fs_filesz(fd) + offset;
      break;
    default:
      // 无效的whence参数
      panic("fs_lseek: invalid whence (%d)", whence);
      assert(0);
  }
  Log("seek success!");
  return new_offset;
}

int fs_close(int fd) {
  // 检查文件描述符是否有效
  if (fd < 0 || fd >= NR_FILES) {
    Log("fs_close: invalid file descriptor %d", fd);
    return -1;
  }
  Log("fs_close: file '%s' (fd=%d) closed", file_table[fd].name, fd);
  return 0;
}
