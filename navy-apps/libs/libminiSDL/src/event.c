#include <NDL.h>
#include <SDL.h>
#include <string.h>

#define keyname(k) #k,

static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};

int SDL_PushEvent(SDL_Event *ev) {
  return 0;
}

int SDL_PollEvent(SDL_Event *ev) {
  char buf[64];
  if (NDL_PollEvent(buf, sizeof(buf)) == 0) {
    return 0;
  }
  char first[5] = {0};
  char rest[20] = {0};
  sscanf(buf, "%s %s", first, rest);
  if (strcmp(first, "kd") == 0) {
    ev->type = SDL_KEYDOWN;
  } else {
    ev->type = SDL_KEYUP;
  }

  int len = sizeof(keyname) / sizeof(keyname[0]);
  for (int i = 0; i < len; i ++) {
    if (strcmp(rest, keyname[i]) == 0) {
      ev->key.keysym.sym = i;
      break;
    }
  }
  return 1;
}

int SDL_WaitEvent(SDL_Event *event) {  
  while (1) {
    if (SDL_PollEvent(event) != 0) break;
  }
  return 1; 
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
  return NULL;
}
