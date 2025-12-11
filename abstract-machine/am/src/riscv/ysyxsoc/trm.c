#include <am.h>
#include <klib-macros.h>
#include <klib.h>
#include <stdint.h>

extern char _heap_start;
extern char _heap_end;

typedef int (*mainfunc_t)(const char *args);

int main(const char *args);

extern char _pmem_start;
#define PMEM_SIZE (8 * 1024)
#define PMEM_END  ((uintptr_t)&_pmem_start + PMEM_SIZE)

Area heap = RANGE(&_heap_start, &_heap_end);
static const char mainargs[MAINARGS_MAX_LEN] = TOSTRING(MAINARGS_PLACEHOLDER); // defined in CFLAGS

#define SERIAL_PORT 0x10000000

void putch(char ch) {
	*(uint8_t *)(SERIAL_PORT + 0x00) = ch;
}

void halt(int code) {	
asm volatile("mv a0, %0; ebreak" : :"r"(code));
	while (1) {} // make sure no return
}

extern char _data, _edata,_text, _etext;
extern char _bss, _ebss;

void _trm_init() {
	memcpy((void *)&_data, (void *)&_text, (uintptr_t)&_etext - (uintptr_t)&_text);
	memset((void *)&_bss, 0, (uintptr_t)&_ebss - (uintptr_t)&_bss);
	int ret = main(mainargs);
	halt(ret);
}
