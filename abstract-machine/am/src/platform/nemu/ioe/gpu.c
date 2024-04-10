#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)
int    printf    (const char *format, ...);
void __am_gpu_config(AM_GPU_CONFIG_T *cfg);
void __am_gpu_init() {
  // int i;
  // AM_GPU_CONFIG_T cfg;
  // __am_gpu_config(&cfg);
  // int w = cfg.width;  // TODO: get the correct width
  // int h = cfg.height;  // TODO: get the correct height
  

  // uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  // for (i = 0; i < w * h; i ++) fb[i] = i;
  // outl(SYNC_ADDR, 1);
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  uint32_t vga_wh = inl(VGACTL_ADDR);
  int w = vga_wh >> 16;
  int h = vga_wh & 0xffff;
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = w, .height = h,
    .vmemsz = 0
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  //printf("enter fbdraw\n");
  int x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;
  if(!ctl->sync && (w == 0 || h == 0))
    return;
  uint32_t *pix = ctl->pixels;
  uintptr_t vmm_addr = FB_ADDR;
  uint32_t *vmm = (uint32_t *)vmm_addr;
  int vga_w = inl(VGACTL_ADDR) >> 16;
  for(int i = y; i < y + h; i++)
  {
    for(int j = x; j < x + w; j++)
    {
      vmm[i * vga_w + j] = pix[(i - y) * w + j - x];
    }
  }
  if (ctl->sync) {
    //printf("sync image\n");
    outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
