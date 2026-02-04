#include <stdio.h>
#include <stdint.h>

void __asan_load4_noabort(uintptr_t addr) {
    printf("ASan: Illegal read access at address %08x\n", (uintptr_t)addr);
    while(1);
}
void __asan_handle_no_return() { }
