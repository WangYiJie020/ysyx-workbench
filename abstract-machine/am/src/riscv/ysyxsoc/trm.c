#include <am.h>
#include <klib-macros.h>
#include <stdint.h>

#include "soc_devreg.h"

#define UART_CLOCK_HZ 25000000u
#define UART_BAUD_RATE 115200u
#define UART_DIVISOR                                                          \
  ((UART_CLOCK_HZ + UART_BAUD_RATE * 8u) / (UART_BAUD_RATE * 16u))

extern char __heap_start;
extern char __heap_end;
int main(const char *args);

Area heap = RANGE(&__heap_start, &__heap_end);
static const char mainargs[MAINARGS_MAX_LEN] = TOSTRING(MAINARGS_PLACEHOLDER);

static void init_serial(void) {
  *UART_LCR = UART_LCR_DLAB | UART_LCR_8N1;
  *UART_DL_MSB = (UART_DIVISOR >> 8) & 0xffu;
  *UART_DL_LSB = UART_DIVISOR & 0xffu;
  *UART_LCR = UART_LCR_8N1;

  *UART_FIFO_CTRL =
      UART_FIFO_TRIGGER_14 | UART_FIFO_CLEAR_RX | UART_FIFO_CLEAR_TX;
  *UART_IER = 0u;
}

void putch(char ch) {
  while ((*UART_LSR & UART_LSR_THRE) == 0u) {
  }
  *UART_TX = (uint8_t)ch;
}

char try_getch(void) {
  if ((*UART_LSR & UART_LSR_DATA_READY) == 0u) {
    return (char)0xff;
  }
  return (char)*UART_RX;
}

void halt(int code) {
  asm volatile("mv a0, %0; ebreak" : : "r"(code));
  while (1) {
  }
}

void _trm_init(void) {
  init_serial();
  halt(main(mainargs));
}
