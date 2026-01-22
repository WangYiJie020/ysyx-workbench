#include <am.h>
#include <klib-macros.h>
#include <klib.h>
#include <stdint.h>

#include <my_putnum.h>

#include "soc_devreg.h"

extern char _heap_start;
extern char _heap_end;

typedef int (*mainfunc_t)(const char *args);

int main(const char *args);

extern char _pmem_start;
#define PMEM_SIZE (8 * 1024)
#define PMEM_END ((uintptr_t) & _pmem_start + PMEM_SIZE)

Area heap = RANGE(&_heap_start, &_heap_end);
static const char mainargs[MAINARGS_MAX_LEN] =
    TOSTRING(MAINARGS_PLACEHOLDER); // defined in CFLAGS

#define BOOT_TEXT __attribute__((section(".boot_text")))
#define BOOT_RODATA __attribute__((section(".boot_rodata")))

BOOT_TEXT void init_serial() {

  // set UART to 8 bits, no parity, one stop bit
  // 0x3 = 0b11 : Select each character 8 bits
  // 0x80 = 0b10000000 : Divisor Latch Access bit
  *UART_LCR = 0x83u;
  if (*UART_LCR != 0x83u) {
    halt(-114514);
  }

  // set baud rate to 115200
  *UART_DL_MSB = 0;
  *UART_DL_LSB = 1;
  // clear DLAB bit
  *UART_LCR = 0x3;
  // enable FIFO with 14-byte threshold
  *UART_FIFO_CTRL = (0x3 << 6);
  // disable all interrupts
  *UART_IER = 0x0;
}

void putch(char ch) {
  while (!(*UART_LSR & 0x20)) {
  }
  *(volatile uint8_t *)(UART_BASE + 0x00) = ch;
}

BOOT_TEXT void boot_putch(char ch) {
  while (!(*UART_LSR & 0x20)) {
  }
  *(volatile uint8_t *)(UART_BASE + 0x00) = ch;
}
#define boot_putstr(s) \
  ({ for (const char *p = s; *p; p++) boot_putch(*p); })


void halt(int code) {
  asm volatile("mv a0, %0; ebreak" : : "r"(code));
  while (1) {
  } // make sure no return
}

void print_csr() {
  uint32_t mvendor_id, marchid;
  asm volatile("csrr %0, mvendorid" : "=r"(mvendor_id));
  asm volatile("csrr %0, marchid" : "=r"(marchid));
  char *vendor = (char *)&mvendor_id;
  putstr("mvendor: 0x");
  putnum_base16(mvendor_id);
  putch(' ');
  putch('(');
  putch(vendor[3]);
  putch(vendor[2]);
  putch(vendor[1]);
  putch(vendor[0]);
  putch(')');
  putch('\n');
  putstr("marchid: 0x");
  // base 10 too slow
  putnum_base16(marchid);
  putch('\n');
}

extern char _text_start[], _text_end[];
extern char _rodata_start[], _rodata_end[];
extern char _data_start[], _data_end[];
extern char _bss_start[], _bss_end[];

extern char __text_load_start__[];
extern char __text_size__[];

extern char __rodata_load_start__[];
extern char __rodata_size__[];

extern char __data_load_start__[];
extern char __data_size__[];

extern char __sram_start__[];
extern char __sram_end__[];

extern char __psram_start__[];
extern char __psram_end__[];

typedef int (*entry_func_t)(const char *args);

#define IS_4BYTE_ALIGNED(x) ((((uintptr_t)(x)) & 0x3) == 0)

BOOT_TEXT static void _boot_failed(const char* msg){
	boot_putstr(msg);
	halt(-1);
}
#define _TOSTR(x) #x
#define BOOT_ASSERT(cond) \
	do { \
		if (!(cond)) { \
			char msg[] = "@line " _TOSTR(__LINE__) ": ASSERTION FAILED : " #cond "\n"; \
			_boot_failed(msg); \
		} \
	} while (0)

BOOT_TEXT static void _word_memcpy(uint32_t *dst, const uint32_t *src, size_t wn) {
	for (size_t i = 0; i < wn; i++) {
		dst[i] = src[i];
	}
}

BOOT_TEXT void boot_memcpy(void *dst, const void *src, size_t n) {
	BOOT_ASSERT(IS_4BYTE_ALIGNED(dst));
	BOOT_ASSERT(IS_4BYTE_ALIGNED(src));
	BOOT_ASSERT(IS_4BYTE_ALIGNED(n));
	size_t wn = n / 4;
	_word_memcpy((uint32_t *)dst, (const uint32_t *)src, wn);
}
BOOT_TEXT void boot_clear(void *dst, size_t n) {
	assert(IS_4BYTE_ALIGNED(dst));
	assert(IS_4BYTE_ALIGNED(n));
	size_t wn = n / 4;
	uint32_t *d = (uint32_t *)dst;
	for (size_t i = 0; i < wn; i++) {
		d[i] = 0;
	}
}

BOOT_RODATA const char msg1[] = "test123\n";

BOOT_TEXT void _trm_init() {
  init_serial();

  putstr(msg1);

  boot_memcpy(_text_start, __text_load_start__, (size_t)__text_size__);
  boot_memcpy(_rodata_start, __rodata_load_start__, (size_t)__rodata_size__);
  boot_memcpy(_data_start, __data_load_start__, (size_t)__data_size__);

  boot_clear(_bss_start, (size_t)(_bss_end - _bss_start));

  int ret = main(mainargs);
  halt(ret);
}
