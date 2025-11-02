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

word_t mem[256]={
	0x01400513,
	0x010000e7,
	0x00c000e7,
	0x00100073, // ebreak
	0x00a50513,
	0x00008067
};

bool is_running=true;

void raise_break(){
	is_running=false;
	puts("\n---Verilator raise break signal---\n");
}

uint32_t pmem_read(uint32_t addr){
	return mem[addr/4];
}


static void single_cycle() {
    dut.clk=0;dut.eval();
//	nvboard_update();

	dut.inst=pmem_read(dut.pc);
	printf(">>> pc %08X",dut.pc);
	printf("inst %08X\n",dut.inst);

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

    while(is_running) {
		single_cycle();
    }

    return 0;
}
