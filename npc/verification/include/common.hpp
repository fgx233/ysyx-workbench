#ifndef __COMMON_H__
#define __COMMON_H__


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <assert.h>

#include "macro.hpp"
#include "generated/autoconf.hpp"




// 定义无符号字类型，有符号字类型，以及以16进制格式化打印字
typedef MUXDEF(CONFIG_ISA64, uint64_t, uint32_t) word_t;
typedef MUXDEF(CONFIG_ISA64, int64_t, int32_t)  sword_t;
#define FMT_WORD MUXDEF(CONFIG_ISA64, "0x%016" PRIx64, "0x%08" PRIx32)

// 定义虚拟地址类型
typedef word_t vaddr_t;

// 定义物理地址类型，以及以16进制格式化打印地址
typedef MUXDEF(PMEM64, uint64_t, uint32_t) paddr_t;
#define FMT_PADDR MUXDEF(PMEM64, "0x%016" PRIx64, "0x%08" PRIx32)

// 添加格式化输入宏
#define SCN_WORD MUXDEF(CONFIG_ISA64, "%" SCNx64, "%" SCNx32) //用于格式化输入word_t类型的参数(十六进制)
#define SCN_SWORD MUXDEF(CONFIG_ISA64, "%" SCNd64, "%" SCNd32) //用于格式化输入signed word_t类型参数(十进制)
#define SCN_UWORD MUXDEF(CONFIG_ISA64, "%" SCNu64, "%" SCNu32) //用于格式化输入unsigned word_t类型参数(十进制)
#define SCN_PADDR MUXDEF(PMEM64, "%" SCNx64, "%" SCNx32) //用于格式化输入word_t类型的参数

// 添加有符号字类型格式化输出宏
#define FMT_SWORD MUXDEF(CONFIG_ISA64, "0d%" PRId64, "0d%" PRId32)
#define FMT_UWORD MUXDEF(CONFIG_ISA64, "%" PRIu64, "%" PRIu32)

#include "debug.hpp"

#endif