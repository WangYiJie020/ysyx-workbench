#include <stdint.h>
#include <assert.h>
#include <stdio.h>

#include "btrace_pack.h"

struct btrace_pack_imp {
	bool is_create = false;
	FILE* fp = nullptr;
	uint32_t count = 0;
	btrace_record_t cur = {0, 0, 0};
};

// in file
// first 4 bytes: count
// then count records, each record is 12 bytes (pc, code, nxt_pc)

#define AssertWr(fp,item) do {\
	assert(fwrite(item, sizeof(*item), 1, fp) == 1);\
} while(0)
#define AssertRd(fp,item) do {\
	assert(fread(item, sizeof(*item), 1, fp) == 1);\
} while(0)

btrace_pack_t btrace_pack_create(const char* path) {
	btrace_pack_t pack = new btrace_pack_imp;
	pack->fp = fopen(path, "wb");
	assert(pack->fp);
	AssertWr(pack->fp, &pack->count);
	pack->is_create = true;
	return pack;
}

btrace_pack_t btrace_pack_open(const char* path) {
	btrace_pack_t pack = new btrace_pack_imp;
	pack->fp = fopen(path, "rb");
	assert(pack->fp);
	AssertRd(pack->fp, &pack->count);
	return pack;
}

void btrace_pack_push(btrace_pack_t pack, const btrace_record_t* record) {
	AssertWr(pack->fp, record);
	pack->count++;
}

void btrace_pack_pop(btrace_pack_t pack, btrace_record_t* record) {
	assert(pack->count > 0);
	AssertRd(pack->fp, record);
	pack->count--;
}

void btrace_pack_close(btrace_pack_t pack) {
	assert(pack->fp);
	if (pack->is_create) {
		fseek(pack->fp, 0, SEEK_SET);
		AssertWr(pack->fp, &pack->count);
	}
	fclose(pack->fp);
	delete pack;
}


