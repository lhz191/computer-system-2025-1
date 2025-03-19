#include <am.h>
#include <x86.h>

#define RTC_PORT 0x48   // Note that this is not standard
static unsigned long boot_time;

#define I8042_DATA_PORT 0x60
#define I8042_STATUS_PORT 0x64


/*Pa2.3 初始化*/
void _ioe_init() {
  boot_time = inl(RTC_PORT);
}

/*Pa2.3 获取开机时间*/
unsigned long _uptime() {
  unsigned long now = inl(RTC_PORT);
  return now - boot_time;
}

uint32_t* const fb = (uint32_t *)0x40000;

_Screen _screen = {
  .width  = 400,
  .height = 300,
};

extern void* memcpy(void *, const void *, int);

// void _draw_rect(const uint32_t *pixels, int x, int y, int w, int h) {
//   int i;
//   for (i = 0; i < _screen.width * _screen.height; i++) {
//     fb[i] = i;
//   }
// }

void _draw_rect(const uint32_t *pixels, int x, int y, int w, int h) {
  int cp_bytes = sizeof(uint32_t) * w;
  for (int i = 0; i < h; i++) {
    uint32_t *dst = &fb[(y + i) * _screen.width + x];
    const uint32_t *src = pixels + i * w;
    memcpy(dst, src, cp_bytes);
  }
}

void _draw_sync() {//无需实现
}

int _read_key() {
  if (inb(I8042_STATUS_PORT) & 0x1) {
    return inl(I8042_DATA_PORT);
  }
  return _KEY_NONE;
}
