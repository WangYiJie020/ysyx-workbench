#include <am.h>
#include <klib-macros.h>
#include <stdint.h>

extern char _heap_start;
extern char _pmem_start;
int main(const char *args);

#define PMEM_SIZE (128 * 1024 * 1024)
#define SERIAL_PORT ((volatile uint8_t *)0x10000000u)

Area heap = RANGE(&_heap_start, (uintptr_t)&_pmem_start + PMEM_SIZE);
static const char mainargs[MAINARGS_MAX_LEN] =
    TOSTRING(MAINARGS_PLACEHOLDER);

void putch(char ch) { *SERIAL_PORT = (uint8_t)ch; }
char try_getch(void) { return (char)0xff; }

void halt(int code) {
  asm volatile("mv a0, %0; ebreak" : : "r"(code));
  while (1) {
  }
}

void _trm_init(void) { halt(main(mainargs)); }
