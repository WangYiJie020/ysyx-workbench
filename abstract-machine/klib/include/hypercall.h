#ifndef _HYPERCALL_H__
#define _HYPERCALL_H__

#include <stdbool.h>
#include <stdint.h>

// addi x0, x0, 114
#define __HyperMagicInst 0x07200013
#define __HyperCallSuccess 0

typedef enum {
  // (dst, src, n)
  __HCmd_memcpy = 0x100,
  // (dst, c, n)
  __HCmd_memset,
} __HyperCmd;

intptr_t __HyperCall__(uintptr_t cmd, uintptr_t a1, uintptr_t a2, uintptr_t a3,
                       uintptr_t a4, uintptr_t a5);

#define __HyperCall(cmd, a1, a2, a3, a4, a5)                                   \
  __HyperCall__((uintptr_t)cmd, (uintptr_t)(a1), (uintptr_t)(a2), (uintptr_t)(a3),        \
                (uintptr_t)(a4), (uintptr_t)(a5))

#define __HyperMemcpy(dst, src, n)                                             \
  (__HyperCall(__HCmd_memcpy, dst, src, (void *)n, 0, 0) == __HyperCallSuccess)

#define __HyperMemset(dst, c, n)                                               \
  (__HyperCall(__HCmd_memset, dst, (void *)(uintptr_t)c, (void *)n, 0, 0) ==   \
   __HyperCallSuccess)

#endif
