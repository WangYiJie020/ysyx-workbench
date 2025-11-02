#include <cstdint>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <verilated.h>
#include <verilated_vcd_c.h>

#include "Vtop.h"
#include "Vtop__Dpi.h"
#include <nvboard.h>

#ifndef TOP_NAME
#define TOP_NAME Vtop
#endif

static TOP_NAME dut;

void nvboard_bind_all_pins(TOP_NAME* top);
void nvboard_update();
void nvboard_init(int vga_clk_cycle);

typedef uint32_t word_t;
typedef uint32_t addr_t;

word_t mem[256]={
	0x01400513,
	0x010000e7,
	0x00c000e7,
	0x00100073, // ebreak
	0x00a50513,
	0x00008067
};

bool is_running=true;

extern "C" void raise_break(){
	is_running=false;
	puts("\n---break signal raise---\n");
}

extern "C" int pmem_read(int raddr) {
  	// 总是读取地址为`raddr & ~0x3u`的4字节返回
  	raddr&=~0x3u;
	return mem[raddr>>2];
}
extern "C" void pmem_write(int waddr, int wdata, char wmask) {
	// 总是往地址为`waddr & ~0x3u`的4字节按写掩码`wmask`写入`wdata`
	// `wmask`中每比特表示`wdata`中1个字节的掩码,
	// 如`wmask = 0x3`代表只写入最低2个字节, 内存中的其它字节保持不变
	
	uint8_t* p=(uint8_t*)mem;

	while (wmask) {
		if(wmask&1){
			*p=wdata&0xff;
		}
		p++;
		wmask>>=1;
		wdata>>=8;
	}
}

static void single_cycle() {
    dut.clk=0;dut.eval();
//	nvboard_update();

	printf("@ pc [%08X]:\n",dut.pc);

    dut.clk=1;dut.eval();
	nvboard_update();
}

static void reset(int n) {
    dut.rst = 1;
    while (n -- > 0) single_cycle();
    dut.rst = 0;
}

int main(int argc, char **argv)
{
//	pmem_write(0,0x12345678, 0x3);
//	int res=pmem_read(0);
//	printf("%X",res);
//	return 0;

    nvboard_bind_all_pins(&dut);
    nvboard_init();

    reset(10);

    while(is_running) {
		single_cycle();
    }

	dut.final();

	puts("\n--- simulation end ---\n");

    return 0;
}
