#include <am.h>
#include <klib.h>
#include <klib-macros.h>

#define _AT_SECTION(name) __attribute__((section(name)))

#define CHECK(expr) do { \
	if (!(expr)) { \
		printf("CHECK FAILED: %s\n", #expr); \
		assert(0); \
	} \
} while (0)

typedef struct {
	uint8_t a;
	uint16_t b;
	uint32_t c;
} __attribute__((packed)) dummy_packed_struct;


_AT_SECTION(".bss.extra") static uint8_t _bss_extra_arr[1145];
_AT_SECTION(".bss.extra") static uint8_t _bss_extra_var;
_AT_SECTION(".bss.extra") static dummy_packed_struct _bss_extra_struct;

void test_bss_extra() {
	for (size_t i = 0; i < sizeof(_bss_extra_arr); i++) {
		CHECK(_bss_extra_arr[i] == 0);
	}
	CHECK(_bss_extra_var == 0);
	CHECK(_bss_extra_struct.a == 0);
	CHECK(_bss_extra_struct.b == 0);
	CHECK(_bss_extra_struct.c == 0);
}

_AT_SECTION(".bss") static uint8_t _bss_arr[1145];
_AT_SECTION(".bss") static uint8_t _bss_var;
_AT_SECTION(".bss") static dummy_packed_struct _bss_struct;
void test_bss() {
	for (size_t i = 0; i < sizeof(_bss_arr); i++) {
		CHECK(_bss_arr[i] == 0);
	}
	CHECK(_bss_var == 0);
	CHECK(_bss_struct.a == 0);
	CHECK(_bss_struct.b == 0);
	CHECK(_bss_struct.c == 0);
}

_AT_SECTION(".data") uint8_t _data_arr[1145] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
_AT_SECTION(".data") uint8_t _data_var = 42;
_AT_SECTION(".data") dummy_packed_struct _data_struct = {0x11, 0x2222, 0x33333333};
void test_data() {
	for (size_t i = 0; i < 10; i++) {
		CHECK(_data_arr[i] == (i + 1));
	}
	CHECK(_data_var == 42);
	CHECK(_data_struct.a == 0x11);
	CHECK(_data_struct.b == 0x2222);
	CHECK(_data_struct.c == 0x33333333);
}

int main() {
	test_bss_extra();
	test_bss();
	test_data();
	return 0;
}
