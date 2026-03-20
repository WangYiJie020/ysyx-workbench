#include "am.h"
#include <klib-macros.h>
#include <klib.h>
#include <stdint.h>

#define ATTRIBUTE_NO_SANITIZE_ADDRESS __attribute__((no_sanitize_address))

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t n = 0;
  while (*s) {
    n++;
    s++;
  }
  return n;
}

char *strcpy(char *dst, const char *src) {
  char *pd = dst;
  while (*src) {
    *pd = *src;
    pd++;
    src++;
  }
  *pd = 0;
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
  panic("Not implemented");
}

char *strcat(char *dst, const char *src) {
  char *pd = dst;
  while (*pd)
    pd++;
  while (*src) {
    *pd = *src;
    pd++;
    src++;
  }
  *pd = 0;
  return dst;
}

int strcmp(const char *s1, const char *s2) {
  int tmp;
  while (*s1 && *s2) {
    tmp = (int)*s1 - (int)*s2;
    if (tmp)
      return tmp;
    s1++;
    s2++;
  }
  return (int)*s1 - (int)*s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
	if(n==0)return 0;
	while(n-- > 1 && *s1 && *s2 && *s1 == *s2) {
		s1++;
		s2++;
	}
	return *(unsigned char *)s1 - *(unsigned char *)s2;
  // int tmp;
  // size_t i = 0;
  // while (*s1 && *s2 && i < n) {
  //   i++;
  //   tmp = (int)*s1 - (int)*s2;
  //   if (tmp)
  //     return tmp;
  //   s1++;
  //   s2++;
  // }
  // return (int)*s1 - (int)*s2;
}

ATTRIBUTE_NO_SANITIZE_ADDRESS
void *_no_asan_kmemzero(void *s, size_t n) {
  if (n % 4 == 0) {
		uint32_t *bs = (uint32_t *)s;
		uint32_t *es = bs + n / 4;
		while (bs != es) {
			*bs = 0;
			bs++;
		}
		return s;
  }
  char *bs = (char *)s;
  char *es = bs + n;
  while (bs != es) {
    *bs = 0;
    bs++;
  }
  return s;
}

void *kmemset(void *s, int c, size_t n) {
  char *bs = (char *)s;
  char *es = bs + n;
  while (bs != es) {
    *bs = c;
    bs++;
  }
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  uint8_t *tmp = malloc(n);
  memcpy(tmp, src, n);
  memcpy(dst, tmp, n);
  return dst;
}

static inline void aligned_memcpy(uint32_t *dst, const uint32_t *src, size_t nWord) {
	uint32_t* end = &dst[nWord];
	while (dst != end) {
		*dst = *src;
		dst++;
		src++;
	}
}

static void byte_memcpy(char *out, const char *in, size_t n) {
  const char *ibeg = in;
  char *obeg = out;
  const char *iend = ibeg + n;

  while (ibeg != iend) {
    *obeg = *ibeg;
    ibeg++;
    obeg++;
  }
}

void *kmemcpy(void *out, const void *in, size_t n) {
	if(n == 0) return out;
	if (n <= 4) {
		char *ob = (char *)out;
		const char *ib = (const char *)in;
		while (n--) {
			*ob = *ib;
			ob++;
			ib++;
		}
		return out;
	}
	uint32_t nWord = n >> 2;
	char* bout = (char *)out;
	const char* bin = (const char *)in;

	assert(0);

	uint32_t outHeadUnalignedBytes = (uintptr_t)out & 0x3;
	uint32_t inHeadUnalignedBytes = (uintptr_t)in & 0x3;
	if (outHeadUnalignedBytes == inHeadUnalignedBytes) {
		if (outHeadUnalignedBytes) {
			byte_memcpy(bout, bin, outHeadUnalignedBytes);
			bout += outHeadUnalignedBytes;
			bin += outHeadUnalignedBytes;
		}
		aligned_memcpy((uint32_t *)bout, (const uint32_t *)bin, nWord);
		bout += nWord;
		bin += nWord;
		uint32_t tailBytes = n & 0x3;
		byte_memcpy(bout, bin, tailBytes);
	} else {
		byte_memcpy(bout, bin, n);
	}
  return out;
}

#ifndef KASAN_ENABLED
#undef memset
void* memset(void *s, int c, size_t n) {
	return kmemset(s, c, n);
}
#undef memcpy
void* memcpy(void *out, const void *in, size_t n) {
	return kmemcpy(out, in, n);
}
#endif

int memcmp(const void *s1, const void *s2, size_t n) {
  const char *bs1 = (const char *)s1;
  const char *bs2 = (const char *)s2;
  const char *es1 = bs1 + n;
  while (bs1 != es1) {
    int tmp = (*bs1 - *bs2);
    if (tmp)
      return tmp;
    bs1++;
    bs2++;
  }
  return 0;
}

#endif
