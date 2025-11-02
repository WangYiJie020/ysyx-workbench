#include <cstdint>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <verilated.h>
#include <verilated_vcd_c.h>

#include "Vtop.h"
#include <nvboard.h>

#ifndef TOP_NAME
#define TOP_NAME Vtop
#endif

static TOP_NAME dut;

void nvboard_bind_all_pins(TOP_NAME* top);
void nvboard_update();
void nvboard_init(int vga_clk_cycle);

typedef uint32_t word_t;

word_t mem[256]={
	0x01400513,
	0x010000e7,
	0x00c000e7,
	0x00c00067,
	0x00a50513,
	0x00a50513
};

uint32_t pmem_read(uint32_t addr){
	return mem[addr];
}

static void single_cycle() {
    dut.clk=0;dut.eval();
//	nvboard_update();

	dut.inst=pmem_read(dut.pc);

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

    nvboard_bind_all_pins(&dut);
    nvboard_init();

    reset(10);

	int cnt=0;
    while(1) {
		single_cycle();
		printf("pc %08X\n",dut.pc);
		cnt++;
		if(cnt>20)break;
    }

    return 0;
}
