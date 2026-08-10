#ifndef __UTILS_H__
#define __UTILS_H__

#include <common.hpp>
#include <cpu.hpp>
// ----------- state -----------

enum { NPC_RUNNING, NPC_STOP, NPC_END, NPC_ABORT, NPC_QUIT };

typedef struct {
  int state;
  paddr_t halt_pc;
  uint32_t halt_ret;
} NPCState;

extern NPCState npc_state;



// ----------- log -----------

#define ANSI_FG_BLACK   "\33[1;30m"
#define ANSI_FG_RED     "\33[1;31m"
#define ANSI_FG_GREEN   "\33[1;32m"
#define ANSI_FG_YELLOW  "\33[1;33m"
#define ANSI_FG_BLUE    "\33[1;34m"
#define ANSI_FG_MAGENTA "\33[1;35m"
#define ANSI_FG_CYAN    "\33[1;36m"
#define ANSI_FG_WHITE   "\33[1;37m"
#define ANSI_BG_BLACK   "\33[1;40m"
#define ANSI_BG_RED     "\33[1;41m"
#define ANSI_BG_GREEN   "\33[1;42m"
#define ANSI_BG_YELLOW  "\33[1;43m"
#define ANSI_BG_BLUE    "\33[1;44m"
#define ANSI_BG_MAGENTA "\33[1;45m"
#define ANSI_BG_CYAN    "\33[1;46m"
#define ANSI_BG_WHITE   "\33[1;47m"
#define ANSI_NONE       "\33[0m"

#define ANSI_FMT(str, fmt) fmt str ANSI_NONE

extern bool log_enable(); \

#define log_write(...) IFDEF(CONFIG_TARGET_NATIVE_ELF, \
  do { \
    extern FILE* log_fp; \
    if (log_enable() && log_fp != NULL) { \
      fprintf(log_fp, __VA_ARGS__); \
      fflush(log_fp); \
    } \
  } while (0) \
)

#define _Log(...) \
  do { \
    printf(__VA_ARGS__); \
    log_write(__VA_ARGS__); \
  } while (0)



// src/utils/log.cpp
void init_log(const char *log_file);

// src/utils/timer.cpp
void init_rand();
uint64_t get_time();

// src/utils/state.cpp
int is_exit_status_bad();

// src/utils/disasm.cpp
void init_disasm();

// src/utils/iringbuf.cpp
void iringbuf_insert(const char *log);

void iringbuf_print();

// src/utils/ftrace.cpp
void init_ftrace(const char *elf_flie);
void ftrace_check(Decode *s);

// src/utils/dut.cpp
void init_difftest(char *ref_so_file, long img_size, int port);
void difftest_step(vaddr_t pc);
void difftest_skip_ref();
void difftest_skip_ref_next();
#endif
