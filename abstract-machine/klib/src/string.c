#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  int i = 0;
  while (*(s + i) != '\0') {
    i ++;
  }
  return (size_t)i;
}

char *strcpy(char *dst, const char *src) {
  char *d = dst;
  while (*src != '\0') {
    *dst++ = *src++;
  }
  *dst = *src;
  return d;
}

char *strncpy(char *dst, const char *src, size_t n) {
  panic("Not implemented");
}

char *strcat(char *dst, const char *src) {
  char *d = dst;
  int len = (int)strlen(dst);
  strcpy(dst + len, src);
  return d;
}

int strcmp(const char *s1, const char *s2) {
  int i = 0;
  for (i = 0; (*(s1 + i) != '\0') || (*(s2 + i) != '\0'); i++) {
    if (*(s1 + i) != *(s2 + i)) {
      return (uint8_t)*(s1 + i) - (uint8_t)*(s2 + i);
    }
  }
  return (uint8_t)*(s1 + i) - (uint8_t)*(s2 + i);
}

int strncmp(const char *s1, const char *s2, size_t n) {
  panic("Not implemented");
}

void *memset(void *s, int c, size_t n) {
  void *r = s;
  unsigned char *ps = s;
  unsigned char pc = c;
  while (n-- > 0) {
    *(ps++) = pc; 
  }
  return r;
}

void *memmove(void *dst, const void *src, size_t n) {
  panic("Not implemented");
}

void *memcpy(void *out, const void *in, size_t n) {
  panic("Not implemented");
}

int memcmp(const void *s1, const void *s2, size_t n) {
  const unsigned char *ps1 = s1;
  const unsigned char *ps2 = s2;
  while (n-- > 0) {
    if (*ps1 == *ps2) {
      ps1++;
      ps2++;
      continue;
    } else {
      return *ps1 - *ps2;
    }
  }
  return 0;
}

#endif
