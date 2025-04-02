#include <klib.h>

size_t strlen(const char *s) {
  const char *p = s;
  while (*p) p++;
  return p - s;
}

char* strcpy(char* dst, const char* src) {
  char* ret = dst;
  while ((*dst++ = *src++));
  return ret;
}

char* strncpy(char* dst, const char* src, size_t n) {
  char *ret = dst;
  while (n-- && (*dst++ = *src++));
  return ret;
}

char* strcat(char* dst, const char* src) {
  char *ret = dst;
  while (*dst) dst++;
  while ((*dst++ = *src++));
  return ret;
}

int strcmp(const char* s1, const char* s2) {
  while (*s1 && *s1 == *s2) { s1++; s2++; }
  return (unsigned char)*s1 - (unsigned char)*s2;
}

int strncmp(const char* s1, const char* s2, size_t n) {
  while (n-- && *s1 && *s1 == *s2) { s1++; s2++; }
  return n < 0 ? 0 : (unsigned char)*s1 - (unsigned char)*s2;
}

void* memset(void* v, int c, size_t n) {
  uint8_t *p = (uint8_t*)v;
  while (n--) *p++ = c;
  return v;
}

void* memcpy(void* out, const void* in, size_t n) {
  char* dst = (char*)out;
  const char* src = (const char*)in;
  while (n--) *dst++ = *src++;
  return out;
}
