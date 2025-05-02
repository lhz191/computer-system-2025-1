#include "klib.h"

int printf(const char *fmt, ...) {
  // 简单实现，将输出重定向到_putc
  va_list ap;
  char buf[1024];  // 假设不会超过1024个字符
  va_start(ap, fmt);
  vsnprintf(buf, sizeof(buf), fmt, ap);
  va_end(ap);
  
  for (char *p = buf; *p; p++) {
    _putc(*p);
  }
  
  return 0;
}

int sprintf(char *out, const char *fmt, ...) {
  va_list ap;
  int n;
  va_start(ap, fmt);
  n = vsprintf(out, fmt, ap);
  va_end(ap);
  return n;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  va_list ap;
  int res;
  va_start(ap, fmt);
  res = vsnprintf(out, n, fmt, ap);
  va_end(ap);
  return res;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  return vsnprintf(out, -1, fmt, ap);
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  // 最简单的实现，只支持%d, %s, %c等基本格式
  int cnt = 0;
  char *s;
  int d;
  
  while (*fmt) {
    if (*fmt != '%') {
      if (out && cnt < n) out[cnt] = *fmt;
      cnt++;
      fmt++;
      continue;
    }
    fmt++;
    switch (*fmt) {
      case 's':
        s = va_arg(ap, char*);
        while (*s) {
          if (out && cnt < n) out[cnt] = *s;
          cnt++;
          s++;
        }
        break;
      case 'd':
        d = va_arg(ap, int);
        // 简单处理整数
        if (d < 0) {
          if (out && cnt < n) out[cnt] = '-';
          cnt++;
          d = -d;
        }
        // 简单处理，假设d不会太大
        char buf[16];
        int i = 0;
        do {
          buf[i++] = '0' + (d % 10);
          d /= 10;
        } while (d);
        while (i--) {
          if (out && cnt < n) out[cnt] = buf[i];
          cnt++;
        }
        break;
      case 'c':
        if (out && cnt < n) out[cnt] = (char)va_arg(ap, int);
        cnt++;
        break;
      default:
        if (out && cnt < n) out[cnt] = *fmt;
        cnt++;
    }
    fmt++;
  }
  
  // 确保结果字符串有null终止符
  if (out && cnt < n) out[cnt] = '\0';
  else if (out && n > 0) out[n-1] = '\0';
  
  return cnt;
}
