/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>

#include <generated/autoconf.h>
#include <macro.h>

#ifdef CONFIG_TARGET_AM
#include <klib.h>
#else
#include <assert.h>
#include <stdlib.h>
#endif

#if CONFIG_MBASE + CONFIG_MSIZE > 0x100000000ul
#define PMEM64 1
#endif

typedef MUXDEF(CONFIG_ISA64, uint64_t, uint32_t) word_t;
typedef MUXDEF(CONFIG_ISA64, int64_t, int32_t)  sword_t;
#define FMT_WORD MUXDEF(CONFIG_ISA64, "0x%016" PRIx64, "0x%08" PRIx32)

typedef word_t vaddr_t;
typedef MUXDEF(PMEM64, uint64_t, uint32_t) paddr_t;
#define FMT_PADDR MUXDEF(PMEM64, "0x%016" PRIx64, "0x%08" PRIx32)
typedef uint16_t ioaddr_t;

//添加格式化输入宏
#define SCN_WORD MUXDEF(CONFIG_ISA64, "%" SCNx64, "%" SCNx32) //用于格式化输入word_t类型的参数(十六进制)
#define SCN_SWORD MUXDEF(CONFIG_ISA64, "%" SCNd64, "%" SCNd32) //用于格式化输入signed word_t类型参数(十进制)
#define SCN_UWORD MUXDEF(CONFIG_ISA64, "%" SCNu64, "%" SCNu32) //用于格式化输入unsigned word_t类型参数(十进制)
#define SCN_PADDR MUXDEF(PMEM64, "%" SCNx64, "%" SCNx32) //用于格式化输入word_t类型的参数

//添加有符号字类型格式化输出宏
#define FMT_SWORD MUXDEF(CONFIG_ISA64, "0d%" PRId64, "0d%" PRId32)
#define FMT_UWORD MUXDEF(CONFIG_ISA64, "%" PRIu64, "%" PRIu32)
#include <debug.h>

#endif
