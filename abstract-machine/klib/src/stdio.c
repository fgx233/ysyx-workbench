// #include <am.h>
// #include <klib.h>
// #include <klib-macros.h>
// #include <stdarg.h>

// #if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

// // int int_to_str(char *out, int num, int width, bool fill_zero) {
// //   int n = 0;
// //   int ori_num = num;

// //   if (num < 0) {
// //     out[n++] = '-';
// //     num = -num;
// //   }

// //   char tmp[11];
// //   int i = 0;
// //   while (num > 0) {
// //     tmp[i++] = '0' + num % 10;
// //     num /= 10;
// //   }

// //   if (i == 0) {
// //     tmp[i++] = '0';
// //   }

// //   if (width != -1) {
// //     int num_width = ori_num < 0? i + 1: i;
// //     while (num_width < width) {
// //       if (fill_zero == true) {
// //         out[n++] = '0';
// //       } else {
// //         out[n++] = ' ';
// //       }
// //       num_width++;
// //     }
// //   }

// //   while (i > 0) {
// //     i--;
// //     out[n++] = tmp[i];
// //   }

// //   return n;
// // }

// // int uint_to_str(char *out, unsigned int num, int width, bool fill_zero) {
// //   int n = 0;

// //   char tmp[11];
// //   int i = 0;
// //   while (num > 0) {
// //     tmp[i++] = '0' + num % 10;
// //     num /= 10;
// //   }

// //   if (i == 0) {
// //     tmp[i++] = '0';
// //   }

// //   if (width != -1) {
// //     int num_width = i;
// //     while (num_width < width) {
// //       if (fill_zero == true) {
// //         out[n++] = '0';
// //       } else {
// //         out[n++] = ' ';
// //       }
// //       num_width++;
// //     }
// //   }

// //   while (i > 0) {
// //     i--;
// //     out[n++] = tmp[i];
// //   }

// //   return n;
// // }

// // int hex_to_str(char *out, unsigned int num, int width, bool fill_zero) {
// //   int n = 0;

// //   const char map[17] = "0123456789abcdef";
// //   char temp[11];
// //   int i = 0;
// //   while (num > 0) {
// //     temp[i++] = map[num % 16];
// //     num /= 16;
// //   }

// //   if (i == 0) {
// //     temp[i++] = map[0];
// //   }

// //   if (width != -1) {
// //     int num_width = i;
// //     while (num_width < width) {
// //       if (fill_zero == true) {
// //         out[n++] = '0';
// //       } else {
// //         out[n++] = ' ';
// //       }
// //       num_width++;
// //     }
// //   }

// //   while (i > 0) {
// //     i--;
// //     out[n++] = temp[i];
// //   }

// //   return n;
// // }

// // int printf(const char *fmt, ...) {
// //   char buf[1024];
// //   va_list ap;
// //   va_start(ap, fmt);
// //   int ret = vsnprintf(buf, sizeof(buf), fmt, ap);
  
// //   for (int i = 0; i < ret; i++) {
// //     putch(buf[i]);
// //   }

// //   va_end(ap);

// //   return ret;
// // }

// // int vsprintf(char *out, const char *fmt, va_list ap) {
// //   return vsnprintf(out, 65536, fmt, ap);
// // }

// // int sprintf(char *out, const char *fmt, ...) {
// //   va_list ap;
// //   va_start(ap, fmt);
// //   int ret = vsnprintf(out, 65536, fmt, ap);
// //   va_end(ap);
// //   return ret;
// // }

// // int snprintf(char *out, size_t n, const char *fmt, ...) {
// //   va_list ap;
// //   va_start(ap, fmt);
// //   int ret = vsnprintf(out, n, fmt, ap);
// //   va_end(ap);
// //   return ret;
// // }

// // int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
// //   int len = 0;
// //   int p = 0;
// //   while (fmt[p] != '\0') {
// //     if (fmt[p] != '%') {
// //       out[len++] = fmt[p++];
// //     } else {
// //       p++;
// //       bool fill_zero = false;
// //       int width = -1;
// //       if (fmt[p] == '0') {
// //         fill_zero = true;
// //         width = 0;
// //         p++;
// //       }

// //       while (fmt[p] >= '0' && fmt[p] <= '9') {
// //         width = width * 10 + fmt[p] - '0';
// //         p++;
// //       }

// //       if (fmt[p] == 's') {
// //         char *s = va_arg(ap, char *);
// //         while (*s != '\0') {
// //           out[len++] = *s++;
// //         }
// //         p++;
// //       } else if (fmt[p] == 'd') {
// //         int num = va_arg(ap, int);
// //         len += int_to_str(out + len, num, width, fill_zero);
// //         p++;
// //       } else if (fmt[p] == 'x') {
// //         unsigned int num = va_arg(ap, unsigned int);
// //         len += hex_to_str(out + len, num, width, fill_zero);
// //         p++;
// //       } else if (fmt[p] == 'u') {
// //         unsigned int num = va_arg(ap, unsigned int);
// //         len += uint_to_str(out + len, num, width, fill_zero);
// //         p++;
// //       } else if (fmt[p] == 'c') {
// //         char c = (char)va_arg(ap, int);
// //         out[len++] = c;
// //         p++;
// //       } else {
// //         panic("Not implemented");
// //       }
// //     }
// //   }
// //   out[len] = '\0';
// //   return len;
// // }

// static void get_str_from_dec_helper(char **dst, int dec) {
//   if (dec < 10) {
//     **dst = dec + '0';
//     (*dst)++;
//   } else {
//     get_str_from_dec_helper(dst, dec / 10);
//     **dst = dec % 10 + '0';
//     (*dst)++; 
//   }
// }

// static void get_str_from_udec_helper(char **dst, unsigned int dec) {
//   if (dec < 10) {
//     **dst = dec + '0';
//     (*dst)++;
//   } else {
//     get_str_from_udec_helper(dst, dec / 10);
//     **dst = dec % 10 + '0';
//     (*dst)++; 
//   }
// }

// static void get_str_from_dec(char *dst, int dec) {
//   if (dec == 0) {
//     *dst++ = '0';
//     *dst = '\0';
//     return;
//   }
//   if (dec == -2147483648) {
//     *dst++ = '-';
//     *dst++ = '2';
//     *dst++ = '1';
//     *dst++ = '4';
//     *dst++ = '7';
//     *dst++ = '4';
//     *dst++ = '8';
//     *dst++ = '3';
//     *dst++ = '6';
//     *dst++ = '4';
//     *dst++ = '8';
//     *dst = '\0';
//     return;
//   }
//   if (dec < 0) {
//     *dst = '-';
//     dst++;
//     dec = -dec;
//   }
//   get_str_from_dec_helper(&dst, dec);
//   *dst = '\0';
// }

// static void get_str_from_udec(char *dst, unsigned int dec) {
//   if (dec == 0) {
//     *dst++ = '0';
//     *dst = '\0';
//     return;
//   }
  
//   get_str_from_udec_helper(&dst, dec);
//   *dst = '\0';
// }

// static const char hex_luts[16] = {
//   '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
// };

// static void get_str_from_hex_helper(char **dst, unsigned int hex) {
//   if (hex < 16) {
//     **dst = hex_luts[hex];
//     (*dst)++; 
//   } else {
//     get_str_from_hex_helper(dst, hex / 16);
//     **dst = hex_luts[hex % 16];
//     (*dst)++;
//   }
// }

// static void get_str_from_hex(char *dst, unsigned int hex) {
//   if (hex == 0) {
//     *dst++ = '0';
//     *dst = '\0';
//     return;
//   }
//   get_str_from_hex_helper(&dst, hex);
//   *dst = '\0';
// }

// static void get_str_from_pointer_helper(char **dst, size_t hex) {
//   if (hex < 16) {
//     **dst = hex_luts[hex];
//     (*dst)++; 
//   } else {
//     get_str_from_pointer_helper(dst, hex / 16);
//     **dst = hex_luts[hex % 16];
//     (*dst)++;
//   }
// }

// static void get_str_from_pointer(char *dst, void *pointer) {
//   *dst++ = '0';
//   *dst++ = 'x';
//   size_t hex = (size_t)pointer;
//   if (hex == 0) {
//     *dst++ = '0';
//     *dst = '\0';
//     return;
//   }
//   get_str_from_pointer_helper(&dst, hex);
//   *dst = '\0';
// }

// static void write_char(char **dst, char c, int *lens, int max) {
//   if (*lens + 1 < max) {
//     **dst = c;
//     (*dst)++;
//     (*lens)++;
//     return;
//   }
//   (*lens)++;
//   return;
// }

// static void write_str(char **dst, char *src, int *lens, int max, bool is_fill_zero, int min_width, bool is_neg) {
//   int fill_nums = min_width - (int)strlen(src);
//   bool is_need_fill = fill_nums > 0;

//   if (is_neg && is_fill_zero) {
//     write_char(dst, *src, lens, max);
//     src++;
//   }

//   while ((fill_nums-- > 0) && is_need_fill) {
//     if (*lens + 1 < max) {
//       if (is_fill_zero) {
//         **dst = '0';
//       } else {
//         **dst = ' ';
//       }
//       (*dst)++;
//       (*lens)++;
//     } else {
//       (*lens)++;
//     }
//   }

//   while (*src != '\0') {
//     if (*lens + 1 < max) {
//       **dst = *src;
//       (*dst)++;
//       src++;
//       (*lens)++;
//     } else {
//       src++;
//       (*lens)++;
//     }
//   }
// }

// int printf(const char *fmt, ...) {
//   char buf[1024];
//   va_list ap;
//   va_start(ap, fmt);
//   int ret = vsnprintf(buf, sizeof(buf), fmt, ap);

//   for (int i = 0; (i < sizeof(buf) - 1) && i < ret; i++) {
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

//   const char *scan_pointer = fmt;
//   char *dst_buf = out;
//   int lens = 0;

//   while (*scan_pointer != '\0') {
//     if (*scan_pointer != '%') {
//       write_char(&dst_buf, *scan_pointer, &lens, n);
//       scan_pointer++;
//     } else {
//       scan_pointer++;


//       int min_width = 0;
//       bool is_fill_zero = false;
//       if (*scan_pointer == '0') {
//         is_fill_zero = true;
//         scan_pointer++;
//       }
//       while (*scan_pointer >= '0' && *scan_pointer <= '9') {
//         min_width = min_width * 10 + *scan_pointer - '0';
//         scan_pointer++;
//       }



//       if (*scan_pointer == '%') {
//         write_char(&dst_buf, '%', &lens, n);
//       } else if (*scan_pointer == 'c') {
//         char c = va_arg(ap, int);
//         write_char(&dst_buf, c, &lens, n);
//       } else if (*scan_pointer == 's') {
//         char *s = va_arg(ap, char *);
//         write_str(&dst_buf, s, &lens, n, is_fill_zero, min_width, false);
//       } else if (*scan_pointer == 'd') {
//         char dec_str[12] = {0};
//         int dec = va_arg(ap, int);
//         get_str_from_dec(dec_str, dec);
//         write_str(&dst_buf, dec_str, &lens, n, is_fill_zero, min_width, dec < 0); 
//       } else if (*scan_pointer == 'u') {
//         char udec_str[11] = {0};
//         unsigned int udec = va_arg(ap, unsigned int);
//         get_str_from_udec(udec_str, udec);
//         write_str(&dst_buf, udec_str, &lens, n, is_fill_zero, min_width, false); 
//       } else if (*scan_pointer == 'x') {
//         char hex_str[12] = {0};
//         unsigned int hex = va_arg(ap, unsigned int);
//         get_str_from_hex(hex_str, hex);
//         write_str(&dst_buf, hex_str, &lens, n, is_fill_zero, min_width, false);
//       } else if (*scan_pointer == 'p') {
//         char pointer_str[67] = {0};
//         void *pointer = va_arg(ap, void *);
//         get_str_from_pointer(pointer_str, pointer);
//         write_str(&dst_buf, pointer_str, &lens, n, is_fill_zero, min_width, false);
//       } else {
//         panic("未实现特性");
//       }
//       scan_pointer++;
//     }
//   }
//   if (n > 0) {
//     *dst_buf = '\0';
//   }
  
//   return lens;
// }

// #endif

#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

/* ======================= 输出目的地 ======================= */

// 所有字符都经过 put 输出；len 记录"本应输出"的字符总数（含被截断的部分）
typedef struct Out {
  void (*put)(struct Out *o, char c);
  char *buf;
  size_t cap;
  size_t len;
} Out;

// 写入缓冲区：只写前 cap-1 个字符，给 '\0' 留位置
static void put_buf(Out *o, char c) {
  if (o->len + 1 < o->cap) o->buf[o->len] = c;
  o->len++;
}

// 直接输出到串口
static void put_con(Out *o, char c) {
  putch(c);
  o->len++;
}

/* ======================= 格式说明 ======================= */

// % [标志] [宽度] [.精度] [长度] 转换符
enum { LEN_HH, LEN_H, LEN_NONE, LEN_L, LEN_LL, LEN_Z };

typedef struct {
  bool left;    // '-'  左对齐
  bool plus;    // '+'  正数显示 '+'
  bool space;   // ' '  正数前留一个空格
  bool alt;     // '#'  %#x 加 0x，%#o 保证以 0 开头
  bool zero;    // '0'  用 0 填充宽度
  int width;    // 最小宽度，0 表示未指定
  int prec;     // 精度，-1 表示未指定
  int lmod;     // 长度修饰符
  char conv;    // 转换符
} Spec;

/* ======================= 排版 ======================= */

static void rep(Out *o, char c, int n) {
  while (n-- > 0) o->put(o, c);
}

// 所有转换共用的排版：[左空格] [前缀] [前导0] [主体] [右空格]
static void emit(Out *o, const Spec *s, const char *pre, int npre,
                 int zeros, const char *body, int nbody) {
  int pad = s->width - npre - zeros - nbody;
  if (!s->left) rep(o, ' ', pad);
  for (int i = 0; i < npre; i++) o->put(o, pre[i]);
  rep(o, '0', zeros);
  for (int i = 0; i < nbody; i++) o->put(o, body[i]);
  if (s->left) rep(o, ' ', pad);
}

// d i u o x X p 共用：mag 是绝对值，neg 表示原值是否为负
static void fmt_int(Out *o, const Spec *s, uint64_t mag, bool neg, unsigned base) {
  bool upper = (s->conv == 'X');
  bool is_signed = (s->conv == 'd' || s->conv == 'i');
  const char *dig = upper ? "0123456789ABCDEF" : "0123456789abcdef";

  // 主体：从缓冲区末尾往前填，省去倒序。uint64_t 的八进制最多 22 位
  char buf[24];
  char *body = buf + sizeof(buf);
  if (!(mag == 0 && s->prec == 0)) {          // 精度为 0 且值为 0 时不输出数字
    uint64_t x = mag;
    do { *--body = dig[x % base]; x /= base; } while (x != 0);
  }
  int n = buf + sizeof(buf) - body;

  // 前缀：符号，或 0x/0X
  char pre[2];
  int npre = 0;
  if (is_signed) {
    if (neg) pre[npre++] = '-';
    else if (s->plus) pre[npre++] = '+';      // '+' 优先于 ' '
    else if (s->space) pre[npre++] = ' ';
  }
  if (s->alt && base == 16 && mag != 0) {     // 值为 0 时 %#x 不加 0x
    pre[npre++] = '0';
    pre[npre++] = upper ? 'X' : 'x';
  }

  // 前导 0
  int zeros = s->prec > n ? s->prec - n : 0;  // 精度 = 最少几位数字
  if (s->alt && base == 8 && zeros == 0 && (n == 0 || body[0] != '0')) {
    zeros = 1;                                // %#o 保证以 0 开头
  }
  if (s->zero && !s->left && s->prec < 0) {   // 指定了精度或 '-' 时 '0' 标志无效
    int z = s->width - npre - n;
    if (z > zeros) zeros = z;
  }

  emit(o, s, pre, npre, zeros, body, n);
}

static void fmt_str(Out *o, const Spec *s, const char *str) {
  if (str == NULL) str = "(null)";
  int n = 0;                                  // 有精度时最多读 prec 个字符，不能先 strlen
  while ((s->prec < 0 || n < s->prec) && str[n] != '\0') n++;
  emit(o, s, NULL, 0, 0, str, n);
}

/* ======================= 核心 ======================= */

// va_arg 只在这个函数里调用
static void vformat(Out *o, const char *fmt, va_list ap) {
  while (*fmt != '\0') {
    if (*fmt != '%') {
      o->put(o, *fmt++);
      continue;
    }
    fmt++;  // 跳过 '%'

    Spec s = { .width = 0, .prec = -1, .lmod = LEN_NONE };

    // 1. 标志
    for (;; fmt++) {
      if      (*fmt == '-') s.left  = true;
      else if (*fmt == '+') s.plus  = true;
      else if (*fmt == ' ') s.space = true;
      else if (*fmt == '#') s.alt   = true;
      else if (*fmt == '0') s.zero  = true;
      else break;
    }

    // 2. 宽度：数字，或 '*' 从参数中取（负数表示左对齐）
    if (*fmt == '*') {
      fmt++;
      s.width = va_arg(ap, int);
      if (s.width < 0) { s.left = true; s.width = -s.width; }
    } else {
      while (*fmt >= '0' && *fmt <= '9') s.width = s.width * 10 + (*fmt++ - '0');
    }

    // 3. 精度：'.' 后跟数字或 '*'（负数视为未指定；只有 '.' 时为 0）
    if (*fmt == '.') {
      fmt++;
      s.prec = 0;
      if (*fmt == '*') {
        fmt++;
        s.prec = va_arg(ap, int);
        if (s.prec < 0) s.prec = -1;
      } else {
        while (*fmt >= '0' && *fmt <= '9') s.prec = s.prec * 10 + (*fmt++ - '0');
      }
    }

    // 4. 长度修饰符
    if (*fmt == 'h') {
      fmt++; s.lmod = LEN_H;
      if (*fmt == 'h') { fmt++; s.lmod = LEN_HH; }
    } else if (*fmt == 'l') {
      fmt++; s.lmod = LEN_L;
      if (*fmt == 'l') { fmt++; s.lmod = LEN_LL; }
    } else if (*fmt == 'z') {
      fmt++; s.lmod = LEN_Z;
    }

    // 5. 转换符
    if (*fmt == '\0') break;                  // 格式串以 '%' 结尾，不能越过 '\0'
    s.conv = *fmt++;

    switch (s.conv) {
      case 'd': case 'i': {
        int64_t v;
        switch (s.lmod) {                     // char/short 传参时被提升为 int
          case LEN_HH: v = (signed char)va_arg(ap, int); break;
          case LEN_H:  v = (short)va_arg(ap, int);       break;
          case LEN_L:  v = va_arg(ap, long);             break;
          case LEN_LL: v = va_arg(ap, long long);        break;
          case LEN_Z:  v = (long)va_arg(ap, size_t);     break;
          default:     v = va_arg(ap, int);              break;
        }
        bool neg = v < 0;
        uint64_t mag = neg ? 0 - (uint64_t)v : (uint64_t)v;  // 对最小负数也不会溢出
        fmt_int(o, &s, mag, neg, 10);
        break;
      }

      case 'u': case 'o': case 'x': case 'X': {
        uint64_t v;
        switch (s.lmod) {
          case LEN_HH: v = (unsigned char)va_arg(ap, unsigned int);  break;
          case LEN_H:  v = (unsigned short)va_arg(ap, unsigned int); break;
          case LEN_L:  v = va_arg(ap, unsigned long);                break;
          case LEN_LL: v = va_arg(ap, unsigned long long);           break;
          case LEN_Z:  v = va_arg(ap, size_t);                       break;
          default:     v = va_arg(ap, unsigned int);                 break;
        }
        unsigned base = (s.conv == 'u') ? 10 : (s.conv == 'o') ? 8 : 16;
        fmt_int(o, &s, v, false, base);
        break;
      }

      case 'p': {
        void *p = va_arg(ap, void *);
        if (p == NULL) {
          emit(o, &s, NULL, 0, 0, "(nil)", 5);
        } else {
          s.alt = true;                       // %p 相当于 %#x
          s.conv = 'x';
          fmt_int(o, &s, (uintptr_t)p, false, 16);
        }
        break;
      }

      case 'c': {
        char c = (char)va_arg(ap, int);
        emit(o, &s, NULL, 0, 0, &c, 1);
        break;
      }

      case 's':
        fmt_str(o, &s, va_arg(ap, const char *));
        break;

      case '%':
        o->put(o, '%');
        break;

      default:
        panic("printf: unsupported conversion");
    }
  }
}

/* ======================= 对外接口 ======================= */

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  Out o = { put_buf, out, n, 0 };
  vformat(&o, fmt, ap);
  if (n > 0) out[o.len < n ? o.len : n - 1] = '\0';
  return (int)o.len;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  return vsnprintf(out, (size_t)-1, fmt, ap);   // 不限长度
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int ret = vsnprintf(out, n, fmt, ap);
  va_end(ap);
  return ret;
}

int sprintf(char *out, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int ret = vsprintf(out, fmt, ap);
  va_end(ap);
  return ret;
}

int printf(const char *fmt, ...) {
  Out o = { put_con, NULL, 0, 0 };
  va_list ap;
  va_start(ap, fmt);
  vformat(&o, fmt, ap);
  va_end(ap);
  return (int)o.len;
}

#endif
