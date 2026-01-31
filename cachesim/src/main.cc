#include <itrace_pack.h>
#include <iostream>
#include <cassert>

int main(){
	itrace_pack_t pack = itrace_pack_open("../nemu/itrace_pack.bin");
	assert(pack != NULL);

	uint32_t pc = 0;
	do {
		pc = itrace_pack_pickone(pack);
		if (pc != 0) {
			std::cout << std::hex << pc << std::endl;
		}
	} while (pc != 0);

	return 0;
}
	
