#include "../include/itrace_pack.h"
#include <assert.h>
#include <cstdint>
#include <stdio.h>
#include <stdlib.h>

#define EXTERN_C extern "C"

struct pc_record {
  uint32_t pc;
  // count of consequtive occurrences
  // 0 means [pc]
  // 1 means [pc, pc + 1]
  // etc.
  uint32_t count;
};

struct itrace_pack_imp {
  size_t size;
  FILE *fp;
  pc_record current;
};

EXTERN_C itrace_pack_t itrace_pack_create(const char *filename) {
  itrace_pack_t pack = new itrace_pack_imp;
  pack->current.pc = 0;
  pack->current.count = 0;
  pack->size = 0;
  pack->fp = fopen(filename, "wb");
	assert(pack->fp != nullptr);
  return pack;
}
static void load_one_record(itrace_pack_t pack) {
  auto n = fread(&pack->current, sizeof(pc_record), 1, pack->fp);
  assert(n == 1);
}
static void save_one_record(itrace_pack_t pack) {
  auto n = fwrite(&pack->current, sizeof(pc_record), 1, pack->fp);
  assert(n == 1);
}
static void init_from_file(itrace_pack_t pack) {
	// jump to the end - size_t
	fseek(pack->fp, -sizeof(size_t), SEEK_END);
	// read size
  auto n = fread(&pack->size, sizeof(size_t), 1, pack->fp);
	assert(n == 1);
	// jump to the beginning
	fseek(pack->fp, 0, SEEK_SET);
	// load the first record
	if (pack->size > 0) {
		load_one_record(pack);
	}
}
EXTERN_C itrace_pack_t itrace_pack_open(const char *filename) {
  itrace_pack_t pack = new itrace_pack_imp;
  pack->current.pc = 0;
  pack->current.count = 0;
  pack->size = 0;
  pack->fp = fopen(filename, "rb");
	assert(pack->fp != nullptr);
  init_from_file(pack);
  return pack;
}
EXTERN_C void itrace_pack_close(itrace_pack_t pack) {
	if(pack->current.pc != 0)
		save_one_record(pack);
	fwrite(&pack->size, sizeof(size_t), 1, pack->fp);
  fclose(pack->fp);
  delete pack;
}
EXTERN_C size_t itrace_pack_size(itrace_pack_t pack) { return pack->size; }
EXTERN_C void itrace_pack_add(itrace_pack_t pack, uint32_t pc) {
  if (pack->current.pc + pack->current.count + 1 == pc) {
    pack->current.count += 1;
  } else {
    if (pack->current.pc != 0)
      save_one_record(pack);
    pack->current.pc = pc;
    pack->current.count = 0;
  }
  pack->size += 1;
}
EXTERN_C uint32_t itrace_pack_pickone(itrace_pack_t pack) {
  if (pack->size == 0) {
    return 0;
  }
  uint32_t pc = pack->current.pc + pack->current.count;
  if (pack->current.count == 0) {
    if (pack->size > 1) {
      load_one_record(pack);
    }
  } else {
    pack->current.count -= 1;
  }
  pack->size -= 1;
  return pc;
}
