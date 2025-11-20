#include <stdio.h>

int main(){
	unsigned long cycles;
	asm volatile ("rdcycle %0" : "=r" (cycles));
	printf("Cycle count: %lu\n", cycles);

	return 0;
}
