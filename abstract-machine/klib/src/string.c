#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t len = 0;
  const char *tem = s;
  while (*(tem + len) != '\0')
  {
    len++;
  }

  return len;
}

char *strcpy(char *dst, const char *src) {
  int src_len = strlen(src);

  for(int i = 0; i <= src_len; i++)
  {
    *(dst + i) = *(src + i);
  }
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
  int src_len = strlen(src);

  for (int i = 0; i < n; i++)
  {
    if(i < src_len)
    {
      *(dst + i) = *(src + i);
    }
    else
    {
      *(dst + i) = '\0';
    }
  }
  return dst;
  
}

char *strcat(char *dst, const char *src) {
  int dst_len = strlen(dst);
  int src_len = strlen(src);
  for(int i = 0; i <= src_len; i++)
  {
    *(dst + dst_len + i) = *(src + i);
  }

  return dst;
}

int strcmp(const char *s1, const char *s2) {
  int i = 0;
  int diff = 0;
  
  while (*(s1 + i) != '\0' && *(s2 + i) != '\0')
  {
    diff = *(s1 + i) - *(s2 + i);
    if(diff == 0)
    {
      i++;
      continue;
    }
    else if(diff > 0)
    {
      return 1;
    }
    else
    {
      return -1;
    }
  }
  
  if(*(s1 + i) == '\0' && *(s2 + i) != '\0')
  {
    return -1;
  }

  if(*(s1 + i) != '\0' && *(s2 + i) == '\0')
  {
    return 1;
  }

  if(*(s1 + i) == '\0' && *(s2 + i) == '\0')
  {
    return 0;
  }

  return 0;
  
}

int strncmp(const char *s1, const char *s2, size_t n) {
  int i = 0;
  int diff = 0;
  
  while (i < n)
  {
    if(*(s1 + i) == '\0' && *(s2 + i) != '\0')
    {
      return -1;
    }

    if(*(s1 + i) != '\0' && *(s2 + i) == '\0')
    {
      return 1;
    }

    if(*(s1 + i) == '\0' && *(s2 + i) == '\0')
    {
      return 0;
    }
    diff = *(s1 + i) - *(s2 + i);
    if(diff == 0)
    {
      i++;
      continue;
    }
    else if(diff > 0)
    {
      return 1;
    }
    else
    {
      return -1;
    }
  }

  return 0;
  
}

void *memset(void *s, int c, size_t n) {
  char * tem = s;
  for (int i = 0; i < n; i++)
  {
    *(tem + i) = c;
  }
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  char *out = dst;
  const char *in  = src;

  if(out < in)
  {
    for (int i = 0; i < n; i++)
    {
      *(out + i) = *(in + i);
    }
  }
  if(out > in)
  {
    for(int i = n - 1; i >= 0; i--)
    {
      *(out + i) = *(in + i);
    }
  }
  return dst;
}

void *memcpy(void *out, const void *in, size_t n) {
  char *dst = out;
  const char *src = in;
  for (int i = 0; i < n; i++)
  {
    *(dst + i) = *(src + i);
  }
  
  return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  const char *src1 = s1;
  const char *src2 = s2;
  int   diff = 0;
  for (int i = 0; i < n; i++)
  {
    diff = *(src1 + i) - *(src2 + i);
    if(diff > 0)
    {
      return 1;
    }
    if(diff < 0)
    {
      return -1;
    }

  }

  return 0;
  
}

#endif
