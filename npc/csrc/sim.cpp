#include <VTop.h>
#include <VTop__Dpi.h>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <nvboard.h>

#ifndef TOP_NAME
#endif
TOP_NAME dut;
void nvboard_bind_all_pins(TOP_NAME *top);

static void step_cycle() {
//	printf("-----step-----\n");
	dut.clock = 0;
	dut.eval();

	dut.clock = 1;
	dut.eval();

//	printf("-------------\n");
}
static void reset(int n) {
	dut.reset = 1;
	while (n-- > 0) {
		step_cycle();
	}
	dut.reset = 0;
}

static bool is_running = true;
void raise_ebreak() {
	printf("ebreak raised, stop sim\n");
	is_running = false;
}

typedef uint32_t word_t;
typedef uint32_t addr_t;

#define MADDR_BASE 0x80000000u
word_t mem[600 * 1024 * 1024 / 4] = {
    0x00000297, // auipc t0,0
    //0x00028823, // sb  zero,16(t0)
    0x0102c503, // lbu a0,16(t0)
    0x0102c503, // lbu a0,16(t0)
    0x00100073, // ebreak
    0xdeadbeef, // some data
		0x12345678,
};
uint8_t *mem_atguest(size_t addr) {
  assert(addr >= MADDR_BASE);
  return ((uint8_t *)mem) + addr - MADDR_BASE;
}
word_t guest_to_host(word_t addr) {
  // printf("raw addr %08X\n",addr);
  // assert(addr>=MADDR_BASE);
  word_t res = addr - MADDR_BASE;
  return res;
}

word_t gpr_snap[32];
void gpr_upd(int regno, int data) {
	if (regno == 0)
		return;
	gpr_snap[regno] = data;
}

bool pc_changed = false;
void pc_upd(int pc, int npc) {
//	printf("pc upd pc=%08x npc=%08x\n",pc,npc);
	pc_changed = true;
}

void fetch_inst(int pc, int *out_inst) {
	printf("fetch pc=%08x\n",pc);
	*out_inst= mem[guest_to_host(pc) / 4];
}
void pmem_read(int addr, int *out_data) {
	uint32_t host_aligned = guest_to_host(addr)&(~0x3);
	*out_data = mem[host_aligned/4];
	printf("pmem read addr=%08x get %08X\n",addr,*out_data);
}
void pmem_write(int addr, int data, int mask) {
	printf("pmem write addr=%08x data=%08x mask=%02x\n",addr,data,mask);
	uint32_t host_aligned = guest_to_host(addr)&(~0x3);
	
	uint8_t* p=(uint8_t*)(&mem[host_aligned>>2]);
	uint32_t umask=mask,udata=data;

	while (umask) {
		if(umask&1){
			*p=udata&0xff;
		}
		p++;
		umask>>=1;
		udata>>=8;
	}
}

int main() {
  nvboard_bind_all_pins(&dut);
  //nvboard_init();
	
	reset(10);

  while (is_running) {
		step_cycle();
		if(pc_changed){
			printf("inst finished execution, after pc changed, gprs:\n");
			pc_changed=false;
			printf("gpr snapshot:\n");
			for(int i=0;i<32;i++){
				printf("x%02d=%08x ",i,gpr_snap[i]);
				if(i%8==7)printf("\n");
			}
		}
    //nvboard_update();
  }
  return 0;
}
