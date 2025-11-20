#include <stdio.h>

void print(){
	unsigned long cycles;
asm volatile ("csrr %0, mcycle" : "=r"(cycles));
	printf("Cycle count: %lu\n", cycles);

}

int main(){
	for(int i=0;i<10;i++)
		print();
	
	return 0;
}
