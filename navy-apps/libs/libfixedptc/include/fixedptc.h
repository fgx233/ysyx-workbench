#ifndef _FIXEDPTC_H_
#define _FIXEDPTC_H_

/*
 * fixedptc.h is a 32-bit or 64-bit fixed point numeric library.
 *
 * fixedptc.h 是一个 32 位或 64 位的定点数运算库。
 *
 * The symbol FIXEDPT_BITS, if defined before this library header file
 * is included, determines the number of bits in the data type (its "width").
 * The default width is 32-bit (FIXEDPT_BITS=32) and it can be used
 * on any recent C99 compiler. The 64-bit precision (FIXEDPT_BITS=64) is
 * available on compilers which implement 128-bit "long long" types. This
 * precision has been tested on GCC 4.2+.
 *
 * 如果在包含本头文件之前定义了符号 FIXEDPT_BITS,它将决定数据类型的
 * 位数(即“宽度”)。默认宽度为 32 位(FIXEDPT_BITS=32),任何较新的
 * C99 编译器都能使用。64 位精度(FIXEDPT_BITS=64)只能在实现了 128 位
 * "long long" 类型的编译器上使用,该精度已在 GCC 4.2+ 上测试过。
 *
 * The FIXEDPT_WBITS symbols governs how many bits are dedicated to the
 * "whole" part of the number (to the left of the decimal point). The larger
 * this width is, the larger the numbers which can be stored in the fixedpt
 * number. The rest of the bits (available in the FIXEDPT_FBITS symbol) are
 * dedicated to the fraction part of the number (to the right of the decimal
 * point).
 *
 * 符号 FIXEDPT_WBITS 决定用多少位来表示数的“整数”部分(小数点左侧)。
 * 这个宽度越大,fixedpt 能存储的数就越大。剩下的位(其位数由符号
 * FIXEDPT_FBITS 给出)用来表示数的小数部分(小数点右侧)。
 *
 * Since the number of bits in both cases is relatively low, many complex
 * functions (more complex than div & mul) take a large hit on the precision
 * of the end result because errors in precision accumulate.
 * This loss of precision can be lessened by increasing the number of
 * bits dedicated to the fraction part, but at the loss of range.
 *
 * 由于这两部分的位数都相对较少,许多复杂函数(比除法和乘法更复杂的)
 * 的最终结果精度会受到很大影响,因为精度误差会不断累积。
 * 增加小数部分的位数可以减轻这种精度损失,但代价是表示范围变小。
 *
 * Adventurous users might utilize this library to build two data types:
 * one which has the range, and one which has the precision, and carefully
 * convert between them (including adding two number of each type to produce
 * a simulated type with a larger range and precision).
 *
 * 喜欢折腾的用户可以用这个库构造两种数据类型:一种侧重范围,一种侧重
 * 精度,并在两者之间小心地转换(甚至可以把两种类型的数各取一个相加,
 * 模拟出一个范围更大、精度也更高的类型)。
 *
 * The ideas and algorithms have been cherry-picked from a large number
 * of previous implementations available on the Internet.
 * Tim Hartrick has contributed cleanup and 64-bit support patches.
 *
 * 其中的思路和算法是从互联网上大量已有实现中精选而来的。
 * Tim Hartrick 贡献了代码清理和 64 位支持的补丁。
 *
 * == Special notes for the 32-bit precision ==
 * Signed 32-bit fixed point numeric library for the 24.8 format.
 * The specific limits are -8388608.999... to 8388607.999... and the
 * most precise number is 0.00390625. In practice, you should not count
 * on working with numbers larger than a million or to the precision
 * of more than 2 decimal places. Make peace with the fact that PI
 * is 3.14 here. :)
 *
 * == 关于 32 位精度的特别说明 ==
 * 有符号 32 位定点数库,采用 24.8 格式(24 位整数部分 + 8 位小数部分)。
 * 具体范围是 -8388608.999... 到 8388607.999...,能表示的最小精度是
 * 0.00390625(即 1/256)。实际使用中,不要指望能处理超过一百万的数,
 * 也不要指望精度能超过小数点后 2 位。接受这里 PI 就是 3.14 的事实吧 :)
 */

/*-
 * Copyright (c) 2010-2012 Ivan Voras <ivoras@freebsd.org>
 * Copyright (c) 2012 Tim Hartrick <tim@edgecast.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * ---- 以下为上述许可证的中文译文,仅供参考,以英文原文为准 ----
 *
 * 版权所有 (c) 2010-2012 Ivan Voras <ivoras@freebsd.org>
 * 版权所有 (c) 2012 Tim Hartrick <tim@edgecast.com>
 *
 * 在满足以下条件的前提下,允许以源代码和二进制形式重新分发和使用本软件,
 * 无论是否经过修改:
 * 1. 以源代码形式重新分发时,必须保留上述版权声明、本条件列表以及
 *    下述免责声明。
 * 2. 以二进制形式重新分发时,必须在随附的文档和/或其他材料中复制上述
 *    版权声明、本条件列表以及下述免责声明。
 *
 * 本软件由作者及贡献者“按原样”提供,不提供任何明示或暗示的担保,包括但
 * 不限于对适销性和特定用途适用性的暗示担保。在任何情况下,作者或贡献者
 * 均不对因使用本软件而以任何方式产生的任何直接、间接、偶然、特殊、惩戒性
 * 或后果性损害(包括但不限于替代商品或服务的采购;使用损失、数据丢失或
 * 利润损失;或业务中断)承担责任,无论其成因如何,也无论基于何种责任
 * 理论(合同责任、严格责任或侵权行为,包括过失或其他),即使已被告知
 * 发生此类损害的可能性。
 */

#ifndef FIXEDPT_BITS
#define FIXEDPT_BITS	32
#endif

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if FIXEDPT_BITS == 32
typedef int32_t fixedpt;
typedef	int64_t	fixedptd;
typedef	uint32_t fixedptu;
typedef	uint64_t fixedptud;
#elif FIXEDPT_BITS == 64
typedef int64_t fixedpt;
typedef	__int128_t fixedptd;
typedef	uint64_t fixedptu;
typedef	__uint128_t fixedptud;
#else
#error "FIXEDPT_BITS must be equal to 32 or 64"
#endif

#ifndef FIXEDPT_WBITS
#define FIXEDPT_WBITS	24
#endif

#if FIXEDPT_WBITS >= FIXEDPT_BITS
#error "FIXEDPT_WBITS must be less than or equal to FIXEDPT_BITS"
#endif

#define FIXEDPT_VCSID "$Id$"

#define FIXEDPT_FBITS	(FIXEDPT_BITS - FIXEDPT_WBITS)
#define FIXEDPT_FMASK	(((fixedpt)1 << FIXEDPT_FBITS) - 1)

#define fixedpt_rconst(R) ((fixedpt)((R) * FIXEDPT_ONE + ((R) >= 0 ? 0.5 : -0.5)))
#define fixedpt_fromint(I) ((fixedptd)(I) << FIXEDPT_FBITS)
#define fixedpt_toint(F) ((F) >> FIXEDPT_FBITS)
#define fixedpt_add(A,B) ((A) + (B))
#define fixedpt_sub(A,B) ((A) - (B))
#define fixedpt_fracpart(A) ((fixedpt)(A) & FIXEDPT_FMASK)

#define FIXEDPT_ONE	((fixedpt)((fixedpt)1 << FIXEDPT_FBITS))
#define FIXEDPT_ONE_HALF (FIXEDPT_ONE >> 1)
#define FIXEDPT_TWO	(FIXEDPT_ONE + FIXEDPT_ONE)
#define FIXEDPT_PI	fixedpt_rconst(3.14159265358979323846)
#define FIXEDPT_TWO_PI	fixedpt_rconst(2 * 3.14159265358979323846)
#define FIXEDPT_HALF_PI	fixedpt_rconst(3.14159265358979323846 / 2)
#define FIXEDPT_E	fixedpt_rconst(2.7182818284590452354)

/* fixedpt is meant to be usable in environments without floating point support
 * (e.g. microcontrollers, kernels), so we can't use floating point types directly.
 * Putting them only in macros will effectively make them optional.
 * fixedpt 的设计目标是能在不支持浮点的环境(例如单片机、内核)中使用,
 * 因此不能直接使用浮点类型。只把浮点放在宏里,实际上就让它成了可选项。 */
#define fixedpt_tofloat(T) ((float) ((T)*((float)(1)/(float)(1L << FIXEDPT_FBITS))))

/* Multiplies a fixedpt number with an integer, returns the result.
 * 将一个 fixedpt 数与一个整数相乘,返回结果。 */
static inline fixedpt fixedpt_muli(fixedpt A, int B) {
	return A * B;
}

/* Divides a fixedpt number with an integer, returns the result.
 * 将一个 fixedpt 数除以一个整数,返回结果。 */
static inline fixedpt fixedpt_divi(fixedpt A, int B) {
	return A / B;
}

/* Multiplies two fixedpt numbers, returns the result.
 * 将两个 fixedpt 数相乘,返回结果。 */
static inline fixedpt fixedpt_mul(fixedpt A, fixedpt B) {
	return (fixedptd)A * B / 256;
}


/* Divides two fixedpt numbers, returns the result.
 * 将两个 fixedpt 数相除,返回结果。 */
static inline fixedpt fixedpt_div(fixedpt A, fixedpt B) {
	return (fixedptd)A * 256 / B ;
}

static inline fixedpt fixedpt_abs(fixedpt A) {
	if (A < 0) {
		return ~A + 1;
	}
	return A;
}

static inline fixedpt fixedpt_floor(fixedpt A) {
	return A & 0xffffff00;
}

static inline fixedpt fixedpt_ceil(fixedpt A) {
	uint8_t low = A & 0xff;
	if (low) {
		return fixedpt_floor(A) + fixedpt_fromint(1);
	} else {
		return A;
	}
}

/*
 * Note: adding and substracting fixedpt numbers can be done by using
 * the regular integer operators + and -.
 *
 * 注意:fixedpt 数的加法和减法直接用普通的整数运算符 + 和 - 即可完成。
 */

/**
 * Convert the given fixedpt number to a decimal string.
 * The max_dec argument specifies how many decimal digits to the right
 * of the decimal point to generate. If set to -1, the "default" number
 * of decimal digits will be used (2 for 32-bit fixedpt width, 10 for
 * 64-bit fixedpt width); If set to -2, "all" of the digits will
 * be returned, meaning there will be invalid, bogus digits outside the
 * specified precisions.
 *
 * 将给定的 fixedpt 数转换为十进制字符串。
 * 参数 max_dec 指定生成小数点右侧的多少位十进制数字。若设为 -1,则使用
 * “默认”的小数位数(32 位 fixedpt 宽度为 2 位,64 位宽度为 10 位);
 * 若设为 -2,则返回“全部”数字,这意味着超出指定精度的那些位将是
 * 无效的、虚假的数字。
 */
void fixedpt_str(fixedpt A, char *str, int max_dec);

/* Converts the given fixedpt number into a string, using a static
 * (non-threadsafe) string buffer
 * 将给定的 fixedpt 数转换为字符串,使用一个静态的(非线程安全的)
 * 字符串缓冲区 */
static inline char* fixedpt_cstr(const fixedpt A, const int max_dec) {
	static char str[25];

	fixedpt_str(A, str, max_dec);
	return (str);
}


/* Returns the square root of the given number, or -1 in case of error
 * 返回给定数的平方根,出错时返回 -1 */
fixedpt fixedpt_sqrt(fixedpt A);


/* Returns the sine of the given fixedpt number. 
 * Note: the loss of precision is extraordinary!
 * 返回给定 fixedpt 数的正弦值。
 * 注意:精度损失非常严重! */
fixedpt fixedpt_sin(fixedpt fp);


/* Returns the cosine of the given fixedpt number
 * 返回给定 fixedpt 数的余弦值 */
static inline fixedpt fixedpt_cos(fixedpt A) {
	return (fixedpt_sin(FIXEDPT_HALF_PI - A));
}


/* Returns the tangens of the given fixedpt number
 * 返回给定 fixedpt 数的正切值 */
static inline fixedpt fixedpt_tan(fixedpt A) {
	return fixedpt_div(fixedpt_sin(A), fixedpt_cos(A));
}


/* Returns the value exp(x), i.e. e^x of the given fixedpt number.
 * 返回给定 fixedpt 数 x 的 exp(x) 值,即 e^x。 */
fixedpt fixedpt_exp(fixedpt fp);


/* Returns the natural logarithm of the given fixedpt number.
 * 返回给定 fixedpt 数的自然对数。 */
fixedpt fixedpt_ln(fixedpt x);


/* Returns the logarithm of the given base of the given fixedpt number
 * 返回给定 fixedpt 数以给定底数 base 为底的对数 */
static inline fixedpt fixedpt_log(fixedpt x, fixedpt base) {
	return (fixedpt_div(fixedpt_ln(x), fixedpt_ln(base)));
}


/* Return the power value (n^exp) of the given fixedpt numbers
 * 返回给定 fixedpt 数的幂 n^exp */
static inline fixedpt fixedpt_pow(fixedpt n, fixedpt exp) {
	if (exp == 0)
		return (FIXEDPT_ONE);
	if (n < 0)
		return 0;
	return (fixedpt_exp(fixedpt_mul(fixedpt_ln(n), exp)));
}

#ifdef __cplusplus
}
#endif

#endif
