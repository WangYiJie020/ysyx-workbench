#ifndef __MY_UTILS_H__
#define __MY_UTILS_H__
#include <stddef.h>
#include <stdint.h>

#define RISCV_INST_NOP 0x13u
#define RISCV_INST_RET 0x00008067u // jalr x0, 0(x1)

#define __asm_call_fence_i__() asm volatile("fence.i")

inline uint32_t get_mvendorid(){
	uint32_t mvendor_id;
  asm volatile("csrr %0, mvendorid" : "=r"(mvendor_id));
	return mvendor_id;
}
inline uint32_t get_marchid(){
	uint32_t marchid;
	asm volatile("csrr %0, marchid" : "=r"(marchid));
	return marchid;
}
#endif // __MY_UTILS_H__
