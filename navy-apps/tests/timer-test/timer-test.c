// #include <stdio.h>
// #include <sys/time.h>

// uint64_t get_time_internal() {
//   struct timeval now;
//   gettimeofday(&now, NULL);
//   uint64_t us = now.tv_sec * 1000000 + now.tv_usec;
//   return us;
// }

// int main () {
//   uint64_t us = get_time_internal();
//   while (1) {
//     uint64_t new_us = get_time_internal();
//     if ((new_us - us) >= (1000000 / 2)) {
//       us = new_us;
//       int sec = us / 1000000;
//       printf("现在是第%d秒\n", sec);
//     }
//   }
// }

#include <stdio.h>
#include <NDL.h>

int main ()  {
  NDL_Init(0);
  uint32_t ms = NDL_GetTicks();
  while (1) {
    uint32_t new_ms = NDL_GetTicks();
    if ((new_ms - ms) >= (1000 / 2)) {
      ms = new_ms;
      int sec = ms / 1000;
      printf("现在是第%d秒\n", sec);
    }
  }
}