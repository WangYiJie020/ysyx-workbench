#ifndef _HYPERCALL_H__
#define _HYPERCALL_H__

#include <stdint.h>

// addi x0, x0, 114
#define __HyperMagicInst 0x07200013
#define __HyperCallSuccess 0

typedef enum : uint32_t {
	// (dst, src, n)
  __HCmd_memcpy = 0x100,
	// (dst, c, n)
  __HCmd_memset,
}__HyperCmd;

intptr_t __HyperCall__(__HyperCmd cmd, uintptr_t a1, uintptr_t a2,
                       uintptr_t a3, uintptr_t a4, uintptr_t a5);


inline intptr_t __HyperCall(__HyperCmd cmd, const void *a1, const void *a2, const void *a3,
                            const void *a4, const void *a5) {
  return __HyperCall__(cmd, (uintptr_t)a1, (uintptr_t)a2, (uintptr_t)a3,
                       (uintptr_t)a4, (uintptr_t)a5);
}
#endif
