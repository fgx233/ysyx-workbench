#include <common.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
# define MULTIPROGRAM_YIELD() yield()
#else
# define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
  [AM_KEY_##key] = #key,

static int screen_w = 0;
static int screen_h = 0;

static const char *keyname[256] __attribute__((used)) = {
  [AM_KEY_NONE] = "NONE",
  AM_KEYS(NAME)
};

size_t serial_write(const void *buf, size_t offset, size_t len) {
  int ret = 0;
  
  const char *p = buf;
  for (int i = 0; i < len; i ++) {
    putch(*p);
    p++;
    ret += 1;
  }
  return ret;
}

size_t events_read(void *buf, size_t offset, size_t len) {
  AM_INPUT_KEYBRD_T kbd;
  kbd = io_read(AM_INPUT_KEYBRD);
  if (strcmp(keyname[kbd.keycode], "NONE") == 0) {
    return 0;
  }
  int ret = 0;
  if (kbd.keydown) {
    ret += snprintf(buf, len, "kd");
    len -= ret;
  } else {
    ret += snprintf(buf, len, "ku");
    len -= ret;
  }

  ret += snprintf(buf + ret, len, " %s\n", keyname[kbd.keycode]);
  return ret;
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  AM_GPU_CONFIG_T info;
  info = io_read(AM_GPU_CONFIG);
  return snprintf(buf, len, "WIDTH:%d\nHEIGHT:%d", info.width, info.height);
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  int pixel_num = offset / 4;
  int x = pixel_num % screen_w;
  int y = pixel_num / screen_w;
  io_write(AM_GPU_FBDRAW, x, y, (void *)buf, len / 4, 1, true);
  return len;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();

  screen_w = io_read(AM_GPU_CONFIG).width;
  screen_h = io_read(AM_GPU_CONFIG).height;
}
