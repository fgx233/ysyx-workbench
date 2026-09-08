#include <common.h>

#ifdef CONFIG_ETRACE

void etrace_call(vaddr_t epc, word_t cause, vaddr_t tvec) {
  log_write("[etrace] enter epc=" FMT_PADDR " cause=" FMT_WORD " tvec=" FMT_PADDR "\n", epc, cause, tvec);
}

void etrace_ret(vaddr_t pc, vaddr_t target) {
  log_write("[etrace] mret pc=" FMT_PADDR " target=" FMT_PADDR "\n", pc, target);
}

#endif