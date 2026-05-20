#include <hypercall.h>

#define _STR(x) #x
#define STR(x) _STR(x)

#define _HyperSection __attribute__((section(".hypercall.text")))

__attribute__((noinline,noipa,optimize("O2")))
_HyperSection intptr_t __HyperCall__(uintptr_t cmd, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3,
		uintptr_t arg4, uintptr_t arg5){
  register uint32_t a0 asm("a0") = cmd;
  register uint32_t a1 asm("a1") = arg1;
  register uint32_t a2 asm("a2") = arg2;
  register uint32_t a3 asm("a3") = arg3;
  register uint32_t a4 asm("a4") = arg4;
  register uint32_t a5 asm("a5") = arg5;

  asm volatile(".word " STR(__HyperMagicInst)
               : "+r"(a0)
               : "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5)
               : "memory");

  return (int)a0;
}
