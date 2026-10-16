#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

// int int_to_str(char *out, int num, int width, bool fill_zero) {
//   int n = 0;
//   int ori_num = num;

//   if (num < 0) {
//     out[n++] = '-';
//     num = -num;
//   }

//   char tmp[11];
//   int i = 0;
//   while (num > 0) {
//     tmp[i++] = '0' + num % 10;
//     num /= 10;
//   }

//   if (i == 0) {
//     tmp[i++] = '0';
//   }

//   if (width != -1) {
//     int num_width = ori_num < 0? i + 1: i;
//     while (num_width < width) {
//       if (fill_zero == true) {
//         out[n++] = '0';
//       } else {
//         out[n++] = ' ';
//       }
//       num_width++;
//     }
//   }

//   while (i > 0) {
//     i--;
//     out[n++] = tmp[i];
//   }

//   return n;
// }

// int uint_to_str(char *out, unsigned int num, int width, bool fill_zero) {
//   int n = 0;

//   char tmp[11];
//   int i = 0;
//   while (num > 0) {
//     tmp[i++] = '0' + num % 10;
//     num /= 10;
//   }

//   if (i == 0) {
//     tmp[i++] = '0';
//   }

//   if (width != -1) {
//     int num_width = i;
//     while (num_width < width) {
//       if (fill_zero == true) {
//         out[n++] = '0';
//       } else {
//         out[n++] = ' ';
//       }
//       num_width++;
//     }
//   }

//   while (i > 0) {
//     i--;
//     out[n++] = tmp[i];
//   }

//   return n;
// }

// int hex_to_str(char *out, unsigned int num, int width, bool fill_zero) {
//   int n = 0;

//   const char map[17] = "0123456789abcdef";
//   char temp[11];
//   int i = 0;
//   while (num > 0) {
//     temp[i++] = map[num % 16];
//     num /= 16;
//   }

//   if (i == 0) {
//     temp[i++] = map[0];
//   }

//   if (width != -1) {
//     int num_width = i;
//     while (num_width < width) {
//       if (fill_zero == true) {
//         out[n++] = '0';
//       } else {
//         out[n++] = ' ';
//       }
//       num_width++;
//     }
//   }

//   while (i > 0) {
//     i--;
//     out[n++] = temp[i];
//   }

//   return n;
// }

// int printf(const char *fmt, ...) {
//   char buf[1024];
//   va_list ap;
//   va_start(ap, fmt);
//   int ret = vsnprintf(buf, sizeof(buf), fmt, ap);
  
//   for (int i = 0; i < ret; i++) {
//     putch(buf[i]);
//   }

//   va_end(ap);

//   return ret;
// }

// int vsprintf(char *out, const char *fmt, va_list ap) {
//   return vsnprintf(out, 65536, fmt, ap);
// }

// int sprintf(char *out, const char *fmt, ...) {
//   va_list ap;
//   va_start(ap, fmt);
//   int ret = vsnprintf(out, 65536, fmt, ap);
//   va_end(ap);
//   return ret;
// }

// int snprintf(char *out, size_t n, const char *fmt, ...) {
//   va_list ap;
//   va_start(ap, fmt);
//   int ret = vsnprintf(out, n, fmt, ap);
//   va_end(ap);
//   return ret;
// }

// int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
//   int len = 0;
//   int p = 0;
//   while (fmt[p] != '\0') {
//     if (fmt[p] != '%') {
//       out[len++] = fmt[p++];
//     } else {
//       p++;
//       bool fill_zero = false;
//       int width = -1;
//       if (fmt[p] == '0') {
//         fill_zero = true;
//         width = 0;
//         p++;
//       }

//       while (fmt[p] >= '0' && fmt[p] <= '9') {
//         width = width * 10 + fmt[p] - '0';
//         p++;
//       }

//       if (fmt[p] == 's') {
//         char *s = va_arg(ap, char *);
//         while (*s != '\0') {
//           out[len++] = *s++;
//         }
//         p++;
//       } else if (fmt[p] == 'd') {
//         int num = va_arg(ap, int);
//         len += int_to_str(out + len, num, width, fill_zero);
//         p++;
//       } else if (fmt[p] == 'x') {
//         unsigned int num = va_arg(ap, unsigned int);
//         len += hex_to_str(out + len, num, width, fill_zero);
//         p++;
//       } else if (fmt[p] == 'u') {
//         unsigned int num = va_arg(ap, unsigned int);
//         len += uint_to_str(out + len, num, width, fill_zero);
//         p++;
//       } else if (fmt[p] == 'c') {
//         char c = (char)va_arg(ap, int);
//         out[len++] = c;
//         p++;
//       } else {
//         panic("Not implemented");
//       }
//     }
//   }
//   out[len] = '\0';
//   return len;
// }

static void get_str_from_dec_helper(char **dst, int dec) {
  if (dec < 10) {
    **dst = dec + '0';
    (*dst)++;
  } else {
    get_str_from_dec_helper(dst, dec / 10);
    **dst = dec % 10 + '0';
    (*dst)++; 
  }
}

static void get_str_from_udec_helper(char **dst, unsigned int dec) {
  if (dec < 10) {
    **dst = dec + '0';
    (*dst)++;
  } else {
    get_str_from_udec_helper(dst, dec / 10);
    **dst = dec % 10 + '0';
    (*dst)++; 
  }
}

static void get_str_from_dec(char *dst, int dec) {
  if (dec == 0) {
    *dst++ = '0';
    *dst = '\0';
    return;
  }
  if (dec == -2147483648) {
    *dst++ = '-';
    *dst++ = '2';
    *dst++ = '1';
    *dst++ = '4';
    *dst++ = '7';
    *dst++ = '4';
    *dst++ = '8';
    *dst++ = '3';
    *dst++ = '6';
    *dst++ = '4';
    *dst++ = '8';
    *dst = '\0';
    return;
  }
  if (dec < 0) {
    *dst = '-';
    dst++;
    dec = -dec;
  }
  get_str_from_dec_helper(&dst, dec);
  *dst = '\0';
}

static void get_str_from_udec(char *dst, unsigned int dec) {
  if (dec == 0) {
    *dst++ = '0';
    *dst = '\0';
    return;
  }
  
  get_str_from_udec_helper(&dst, dec);
  *dst = '\0';
}

static const char hex_luts[16] = {
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
};

static void get_str_from_hex_helper(char **dst, unsigned int hex) {
  if (hex < 16) {
    **dst = hex_luts[hex];
    (*dst)++; 
  } else {
    get_str_from_hex_helper(dst, hex / 16);
    **dst = hex_luts[hex % 16];
    (*dst)++;
  }
}

static void get_str_from_hex(char *dst, unsigned int hex) {
  if (hex == 0) {
    *dst++ = '0';
    *dst = '\0';
    return;
  }
  get_str_from_hex_helper(&dst, hex);
  *dst = '\0';
}

static void get_str_from_pointer_helper(char **dst, size_t hex) {
  if (hex < 16) {
    **dst = hex_luts[hex];
    (*dst)++; 
  } else {
    get_str_from_pointer_helper(dst, hex / 16);
    **dst = hex_luts[hex % 16];
    (*dst)++;
  }
}

static void get_str_from_pointer(char *dst, void *pointer) {
  *dst++ = '0';
  *dst++ = 'x';
  size_t hex = (size_t)pointer;
  if (hex == 0) {
    *dst++ = '0';
    *dst = '\0';
    return;
  }
  get_str_from_pointer_helper(&dst, hex);
  *dst = '\0';
}

static void write_char(char **dst, char c, int *lens, int max) {
  if (*lens + 1 < max) {
    **dst = c;
    (*dst)++;
    (*lens)++;
    return;
  }
  (*lens)++;
  return;
}

static void write_str(char **dst, char *src, int *lens, int max, bool is_fill_zero, int min_width, bool is_neg) {
  int fill_nums = min_width - (int)strlen(src);
  bool is_need_fill = fill_nums > 0;

  if (is_neg && is_fill_zero) {
    write_char(dst, *src, lens, max);
    src++;
  }

  while ((fill_nums-- > 0) && is_need_fill) {
    if (*lens + 1 < max) {
      if (is_fill_zero) {
        **dst = '0';
      } else {
        **dst = ' ';
      }
      (*dst)++;
      (*lens)++;
    } else {
      (*lens)++;
    }
  }

  while (*src != '\0') {
    if (*lens + 1 < max) {
      **dst = *src;
      (*dst)++;
      src++;
      (*lens)++;
    } else {
      src++;
      (*lens)++;
    }
  }
}

int printf(const char *fmt, ...) {
  char buf[1024];
  va_list ap;
  va_start(ap, fmt);
  int ret = vsnprintf(buf, sizeof(buf), fmt, ap);

  for (int i = 0; (i < sizeof(buf) - 1) && i < ret; i++) {
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

  const char *scan_pointer = fmt;
  char *dst_buf = out;
  int lens = 0;

  while (*scan_pointer != '\0') {
    if (*scan_pointer != '%') {
      write_char(&dst_buf, *scan_pointer, &lens, n);
      scan_pointer++;
    } else {
      scan_pointer++;


      int min_width = 0;
      bool is_fill_zero = false;
      if (*scan_pointer == '0') {
        is_fill_zero = true;
        scan_pointer++;
      }
      while (*scan_pointer >= '0' && *scan_pointer <= '9') {
        min_width = min_width * 10 + *scan_pointer - '0';
        scan_pointer++;
      }



      if (*scan_pointer == '%') {
        write_char(&dst_buf, '%', &lens, n);
      } else if (*scan_pointer == 'c') {
        char c = va_arg(ap, int);
        write_char(&dst_buf, c, &lens, n);
      } else if (*scan_pointer == 's') {
        char *s = va_arg(ap, char *);
        write_str(&dst_buf, s, &lens, n, is_fill_zero, min_width, false);
      } else if (*scan_pointer == 'd') {
        char dec_str[12] = {0};
        int dec = va_arg(ap, int);
        get_str_from_dec(dec_str, dec);
        write_str(&dst_buf, dec_str, &lens, n, is_fill_zero, min_width, dec < 0); 
      } else if (*scan_pointer == 'u') {
        char udec_str[11] = {0};
        unsigned int udec = va_arg(ap, unsigned int);
        get_str_from_udec(udec_str, udec);
        write_str(&dst_buf, udec_str, &lens, n, is_fill_zero, min_width, false); 
      } else if (*scan_pointer == 'x') {
        char hex_str[12] = {0};
        unsigned int hex = va_arg(ap, unsigned int);
        get_str_from_hex(hex_str, hex);
        write_str(&dst_buf, hex_str, &lens, n, is_fill_zero, min_width, false);
      } else if (*scan_pointer == 'p') {
        char pointer_str[67] = {0};
        void *pointer = va_arg(ap, void *);
        get_str_from_pointer(pointer_str, pointer);
        write_str(&dst_buf, pointer_str, &lens, n, is_fill_zero, min_width, false);
      } else {
        panic("未实现特性");
      }
      scan_pointer++;
    }
  }
  if (n > 0) {
    *dst_buf = '\0';
  }
  
  return lens;
}

#endif
