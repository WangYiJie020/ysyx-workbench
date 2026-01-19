#ifndef __DEV_REG_H__
#define __DEV_REG_H__

#include <stdint.h>

typedef volatile uint8_t* dev_reg8_ptr;
typedef volatile uint16_t* dev_reg16_ptr;
typedef volatile uint32_t* dev_reg32_ptr;

#define DEV_REG8(addr)  ((dev_reg8_ptr)(addr))
#define DEV_REG16(addr) ((dev_reg16_ptr)(addr))
#define DEV_REG32(addr) ((dev_reg32_ptr)(addr))

#define UART_BASE 0x10000000u
#define _UART_R(x)    (DEV_REG8(UART_BASE + (x)))
#define UART_LCR       _UART_R(0x03)
#define UART_DL_LSB    _UART_R(0x00)
#define UART_DL_MSB    _UART_R(0x01)
#define UART_FIFO_CTRL _UART_R(0x02)
#define UART_IER       _UART_R(0x01)
#define UART_LSR       _UART_R(0x05)


#define SPI_BASE 0x10001000u
#define _SPI_R(x)    (DEV_REG32(SPI_BASE + (x)*4))
#define SPI_Rx0     _SPI_R(0)
#define SPI_Rx1     _SPI_R(1)
#define SPI_Rx2     _SPI_R(2)
#define SPI_Rx3     _SPI_R(3)
#define SPI_Tx0     _SPI_R(0)
#define SPI_Tx1     _SPI_R(1)
#define SPI_Tx2     _SPI_R(2)
#define SPI_Tx3     _SPI_R(3)
#define SPI_CTRL    _SPI_R(4)
#define SPI_DIVIDER _SPI_R(5)
#define SPI_SS      _SPI_R(6)

#define _To1b(x) ((x)&0x1)
#define MAKE_SPI_CTRL(ass,ie,lsb,tx_neg,rx_neg,go_bsy,char_len) \
	((_To1b(ass)<<13)    | (_To1b(ie)<<12)    | (_To1b(lsb)<<11)   | \
	 (_To1b(tx_neg)<<10) | (_To1b(rx_neg)<<9) | (_To1b(go_bsy)<<8) | \
	 ((char_len)&0xff))
#define IS_SPI_BUSY() _To1b((*SPI_CTRL)>>8)


#endif // __DEV_REG_H__
