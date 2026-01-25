#ifndef __MY_UTILS_H__
#define __MY_UTILS_H__
#include <stddef.h>
#include <stdint.h>

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
