#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

void itoa(int value, char *string)
{
  int i = 0;
  while(value > 0)
  {
    *(string + i) = value % 10 + '0';
    i++;
    value = value / 10;
  }
  int len = i;
  for(int j = 0; j < len / 2; j++)
  {
    char tmp = *(string + j);
    *(string + j) = *(string + len - 1 - j);
    *(string + len - 1 -j) = tmp;
  }
  *(string + len) = '\0';

}

int printf(const char *fmt, ...) {
  panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  char *dst = out;
  char dec[21] = {0};

  for(int i = 0; *(fmt + i) != '\0'; i++)
  {
    if(*(fmt + i) != '%')
    {
      *dst = *(fmt + i);
      dst++;
      continue;
    }
    i++;
    switch (*(fmt + i))
    {
    case 'd':itoa(va_arg(ap, int), dec);
             strncpy(dst, dec, strlen(dec));
             dst = dst + strlen(dec);
             break;
    case 's':char *buf = va_arg(ap, char *);
             strncpy(dst, buf, strlen(buf));
             dst = dst + strlen(buf);
             break;
    }
  
  }
  *dst = '\0';
  return dst - out;
}

int sprintf(char *out, const char *fmt, ...) {
  va_list ap;
  int i = 0;
  va_start(ap, fmt);
  i = vsprintf(out, fmt, ap);
  va_end(ap);
  return i;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
