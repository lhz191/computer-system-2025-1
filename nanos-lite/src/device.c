#include "common.h"

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

size_t events_read(void *buf, size_t len) {
  return 0;
}

static char dispinfo[128] __attribute__((used));

void dispinfo_read(void *buf, off_t offset, size_t len) {
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

  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention
}
