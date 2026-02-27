#include <common.h>

#ifdef CONFIG_IRINGBUF

#define IRINGBUF_SIZE 16

static char buf[IRINGBUF_SIZE][128];

static int position = 0;

static int old_position = 0;

void iringbuf_insert(const char *log) {
  snprintf(buf[position], 128, "%s", log);
  old_position = position;
  position++;
  position = position % IRINGBUF_SIZE;
}

void iringbuf_print() {
  printf("最近%d条指令:\n", IRINGBUF_SIZE);
  for (int i = 0; i < IRINGBUF_SIZE; i++) {
    if (i == old_position) {
      printf("-->%s\n", buf[i]);
    } else {
      printf("   %s\n", buf[i]);
    }
  }
}

#endif