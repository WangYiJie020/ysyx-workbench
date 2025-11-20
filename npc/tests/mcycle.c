#include <stdio.h>

void print(){
	unsigned long cycles;
	asm volatile ("rdcycle %0" : "=r" (cycles));
	printf("Cycle count: %lu\n", cycles);

}

int main(){
	for(int i=0;i<10;i++)
		print();
	
	return 0;
}
