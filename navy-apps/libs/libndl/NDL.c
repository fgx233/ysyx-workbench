#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <assert.h>

static int evtdev = -1;
static int fbdev = -1;
static int screen_w = 0, screen_h = 0;
static int canvas_w = 0, canvas_h = 0;
static int fix_x = 0, fix_y = 0;

static uint32_t get_time_internal() {
  struct timeval now;
  gettimeofday(&now, NULL);
  uint32_t ms = now.tv_sec * 1000 + now.tv_usec / 1000;
  return ms;
}

uint32_t NDL_GetTicks() {
  static uint32_t boot_time = 0;
  if (boot_time == 0) {
    boot_time = get_time_internal();
    return boot_time;
  }

  uint32_t now = get_time_internal();
  return now - boot_time;
}

int NDL_PollEvent(char *buf, int len) {
  return read(evtdev, buf, len);
}

void NDL_OpenCanvas(int *w, int *h) {
  if (getenv("NWM_APP")) {
    int fbctl = 4;
    fbdev = 5;
    screen_w = *w; screen_h = *h;
    char buf[64];
    int len = sprintf(buf, "%d %d", screen_w, screen_h);
    // let NWM resize the window and create the frame buffer
    write(fbctl, buf, len);
    while (1) {
      // 3 = evtdev
      int nread = read(3, buf, sizeof(buf) - 1);
      if (nread <= 0) continue;
      buf[nread] = '\0';
      if (strcmp(buf, "mmap ok") == 0) break;
    }
    close(fbctl);
  }

  if (*w == 0 && *h == 0) {
    *w = screen_w;
    *h = screen_h;
  }
  if (*w > screen_w || *h > screen_h) {
    printf("长/宽超出限制：w:%d h:%d\n", *w, *h);
    assert(0);
  }
  canvas_w = *w;
  canvas_h = *h;

  fix_x = (screen_w - canvas_w) / 2;
  fix_y = (screen_h - canvas_h) / 2;
  
}

void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
  for (int i = 0; i < h; i ++) {
    lseek(fbdev, ((y + i + fix_y) * screen_w + x + fix_x) * 4, SEEK_SET);
    write(fbdev, pixels + i * w, w * 4);
  }
}

void NDL_OpenAudio(int freq, int channels, int samples) {
}

void NDL_CloseAudio() {
}

int NDL_PlayAudio(void *buf, int len) {
  return 0;
}

int NDL_QueryAudio() {
  return 0;
}

int NDL_Init(uint32_t flags) {
  if (getenv("NWM_APP")) {
    evtdev = 3;
  }
  NDL_GetTicks();
  evtdev = open("/dev/events", 0);

  int fd = open("/proc/dispinfo", 0);
  char info[64] = {0};
  read(fd, info, sizeof(info));
  sscanf(info, "WIDTH:%d HEIGHT:%d", &screen_w, &screen_h);
  close(fd);

  fbdev = open("/dev/fb", 0);

  return 0;
}

void NDL_Quit() {
  close(evtdev);
  close(fbdev);
}
