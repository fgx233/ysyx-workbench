#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int int_to_str(char *out, int num, int width, bool fill_zero) {
  int n = 0;
  int ori_num = num;

  if (num < 0) {
    out[n++] = '-';
    num = -num;
  }

  char tmp[11];
  int i = 0;
  while (num > 0) {
    tmp[i++] = '0' + num % 10;
    num /= 10;
  }

  if (i == 0) {
    tmp[i++] = '0';
  }

  if (width != -1) {
    int num_width = ori_num < 0? i + 1: i;
    while (num_width < width) {
      if (fill_zero == true) {
        out[n++] = '0';
      } else {
        out[n++] = ' ';
      }
      num_width++;
    }
  }

  

  while (i > 0) {
    i--;
    out[n++] = tmp[i];
  }

  return n;
}

int printf(const char *fmt, ...) {
  char buf[1024];
  va_list ap;
  va_start(ap, fmt);
  int ret = vsnprintf(buf, sizeof(buf), fmt, ap);
  
  for (int i = 0; i < ret; i++) {
    putch(buf[i]);
  }

  va_end(ap);

  return ret;
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
      bool fill_zero = false;
      int width = -1;
      if (fmt[p] == '0') {
        fill_zero = true;
        width = 0;
        p++;
      }

      while (fmt[p] >= '0' && fmt[p] <= '9') {
        width = width * 10 + fmt[p] - '0';
        p++;
      }

      if (fmt[p] == 's') {
        char *s = va_arg(ap, char *);
        while (*s != '\0') {
          out[len++] = *s++;
        }
        p++;
      } else if (fmt[p] == 'd') {
        int num = va_arg(ap, int);
        len += int_to_str(out + len, num, width, fill_zero);
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
