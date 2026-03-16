#include <am.h>
#include <nemu.h>

#define KEYDOWN_MASK 0x8000

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
  uint32_t keyboard = inl(KBD_ADDR);
  if ((keyboard & KEYDOWN_MASK) != 0) {
    kbd->keydown = 1;
  } else {
    kbd->keydown = 0;
  }

  kbd->keycode = keyboard & (KEYDOWN_MASK - 1);
}
