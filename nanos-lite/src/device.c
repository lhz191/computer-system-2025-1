#include "common.h"

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

size_t events_read(void *buf, size_t len) {
  int key = _read_key();
  bool is_down = false;
  if(key & 0x8000 ) {
    key ^= 0x8000;
    is_down = true;
  }
  if(key == _KEY_NONE) {
    uint32_t ut = _uptime();
    sprintf(buf, "t %d\n", ut);
  } 
  else {
    sprintf(buf, "%s %s\n", is_down ? "kd" : "ku", keyname[key]);
  }
  
  return strlen(buf);
}


static char dispinfo[128] __attribute__((used));

void dispinfo_read(void *buf, off_t offset, size_t len) {
  // 确保不会读取超过dispinfo字符串长度
  // int disp_len = strlen(dispinfo);
  // if (offset >= disp_len) {
  //   len = 0;
  // } else if (offset + len > disp_len) {
  //   len = disp_len - offset;
  // }
  // 将dispinfo中的数据从offset位置拷贝len字节到buf
  memcpy(buf, dispinfo + offset, len);
}


void fb_write(const void *buf, off_t offset, size_t len) {
  int row = (offset/4) / _screen.width;// 计算行号：将偏移量转换为像素行
  int col = (offset/4) % _screen.width;// 计算列号：将偏移量转换为像素列
  // 绘制矩形：在屏幕上从 (c, r) 开始绘制 len/4 个像素宽度的矩形
  // (uint32_t*)buf：像素数据的起始地址
  // len/4：计算像素数量，因为每个像素占用 4 字节
  // 1：矩形的高度为 1 行
  _draw_rect((uint32_t*)buf, col, row, len/4, 1);
}


void init_device() {
  _ioe_init();

  // 获取屏幕尺寸并格式化信息到dispinfo
  sprintf(dispinfo, "WIDTH:%d\nHEIGHT:%d\n", _screen.width, _screen.height);
  
  Log("Screen size: %d x %d", _screen.width, _screen.height);
}