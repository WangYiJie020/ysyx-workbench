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
0x123450b7,
0x67808093,
0x00001137,
0x23410113,
0x002081b3,
0x12346237,
0x0ac20213,
0x10020213,
0x000182b3,
0x00020333,
0x00000513,
0x00100073,


0x01100093,
0x00100023,
0x02200113,
0x00200223,
0x03300193,
0x00300423,
0x04400213,
0x00400623,
0x00004283,
0x00404303,
0x00804383,
0x00c04403,
0x006284b3,
0x007484b3,
0x008484b3,
0x00902823,
0x06400513,
0x00a02a23,
0x01402583,
0x01002603,
0x00c586b3,
0x00d02c23,
0x05500713,
0x00e00223,
0x00404783,
0x00f02e23,
0x00100073,
};

bool is_running=true;

extern "C" void raise_break(){
	is_running=false;
	puts("\n---break signal raise---\n");
}

extern "C" int pmem_read(int raddr) {
  	// 总是读取地址为`raddr & ~0x3u`的4字节返回
	uint32_t addr=raddr;
  	addr&=~0x3u;
	printf("  $pmem_read try read %08X\n",addr);
	return mem[addr>>2];
}
extern "C" void pmem_write(int waddr, int wdata, char wmask) {
	// 总是往地址为`waddr & ~0x3u`的4字节按写掩码`wmask`写入`wdata`
	// `wmask`中每比特表示`wdata`中1个字节的掩码,
	// 如`wmask = 0x3`代表只写入最低2个字节, 内存中的其它字节保持不变
	uint32_t addr=waddr;
  	addr&=~0x3u;
	printf("  $pmem_write try write %08X\n",addr);
	
	uint8_t* p=(uint8_t*)(&mem[addr>>2]);

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

	if(!dut.rst)printf("@ pc [%08X]:\n",dut.pc);

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

	puts("\n--- Start ---\n");
	
    while(is_running) {
		single_cycle();
    }

	dut.final();

	puts("\n--- simulation end ---\n");

    return 0;
}
