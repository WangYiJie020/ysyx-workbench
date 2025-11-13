#include <capstone/capstone.h>
#include <dlfcn.h>
#include <assert.h>

#define _gv(n) n##_dl
#define _gt(n) n##_func_t

#define _def(st,ret,n,...) \
	typedef ret(*_gt(n))(__VA_ARGS__);\
	st _gt(n) _gv(n) = NULL;

#define _open(n)\
	_gv(n)=(_gt(n))dlsym(dl_handle,#n);\
	assert(_gv(n));

_def(static,size_t, cs_disasm,csh handle, const uint8_t *code, size_t code_size, uint64_t address, size_t count, cs_insn **insn);
_def(static,void, cs_free,cs_insn *insn, size_t count);

static csh handle;


void init_disasm() {

void *dl_handle;
  dl_handle = dlopen("tools/capstone/repo/libcapstone.so.5", RTLD_LAZY);
  assert(dl_handle);

  _def(,cs_err,cs_open,cs_arch arch, cs_mode mode, csh *handle);
  _open(cs_open);
  _open(cs_disasm);
  _open(cs_free);

  cs_arch arch=CS_ARCH_RISCV;
  cs_mode mode=CS_MODE_RISCV32;

  int ret = cs_open_dl(arch, mode, &handle);
  assert(ret == CS_ERR_OK);
}
void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte) {
	cs_insn *insn;
	size_t count = cs_disasm_dl(handle, code, nbyte, pc, 0, &insn);
  assert(count == 1);
  int ret = snprintf(str, size, "%s", insn->mnemonic);
  if (insn->op_str[0] != '\0') {
    snprintf(str + ret, size - ret, "\t%s", insn->op_str);
  }
  cs_free_dl(insn, count);
}
