#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int int_to_str(char *out, int num) {
  int n = 0;

  if (num < 0) {
    out[n++] = '-';
    num = -num;
  }

  if (num == 0) {
    out[n++] = '0';
    return n;
  }

  char tmp[11];
  int i = 0;
  while (num > 0) {
    tmp[i++] = '0' + num % 10;
    num /= 10;
  }

  while (i > 0) {
    i--;
    out[n++] = tmp[i];
  }

  return n;
}

int printf(const char *fmt, ...) {
  panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  return vsnprintf(out, 65536, fmt, ap);
}

int sprintf(char *out, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int ret = vsnprintf(out, 65536, fmt, ap);
  va_end(ap);
  return ret;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int ret = vsnprintf(out, n, fmt, ap);
  va_end(ap);
  return ret;
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  int len = 0;
  int p = 0;
  while (fmt[p] != '\0') {
    if (fmt[p] != '%') {
      out[len++] = fmt[p++];
    } else {
      p++;
      if (fmt[p] == 's') {
        char *s = va_arg(ap, char *);
        while (*s != '\0') {
          out[len++] = *s++;
        }
        p++;
      } else if (fmt[p] == 'd') {
        int num = va_arg(ap, int);
        len += int_to_str(out + len, num);
        p++;
      } else {
        panic("Not implemented");
      }
    }
  }
  out[len] = '\0';
  return len;
}

#endif
