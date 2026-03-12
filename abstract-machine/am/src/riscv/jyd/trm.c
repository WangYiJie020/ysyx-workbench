#include <am.h>
#include <klib-macros.h>
#include <stdio.h>

int main(const char *args);

extern char _heap_start;
extern char _heap_end;

Area heap = RANGE(&_heap_start, &_heap_end);
static const char mainargs[MAINARGS_MAX_LEN] =
    TOSTRING(MAINARGS_PLACEHOLDER); // defined in CFLAGS

// TODO: use real serial port address
#define SERIAL_PORT 0x10000000

void putch(char ch) { *(uint8_t *)(SERIAL_PORT + 0x00) = ch; }
// unsupported now, just return 0xff to indicate no char available
char try_getch() { return 0xff; }

void halt(int code) {
  asm volatile("mv a0, %0; ebreak" : : "r"(code));
  while (1) {
  } // make sure no return
}

void _trm_init() {
  int ret = main(mainargs);
  halt(ret);
}
