#include "sdb.hpp"
#include "sdbc.h"
#include <cassert>

using namespace std;

sdb_debuger sdb_create_debuger(sdb_paddr_t init_pc, sdb_cpu_executor exec, sdb_mem_loader loadmem, sdb_reg_snapshoter shotreg, const char **reg_names, size_t n_reg_names, sdb_disasmsembler disasm, sdb_inst_fetcher fetcher){
	auto regname_vec=	vector<string_view>(reg_names,reg_names+n_reg_names);	
	return (sdb_debuger)(new sdb::debuger(
		init_pc,
		exec,
		loadmem,
		[shotreg](sdb::reg_snapshot_t& snap){
			shotreg(snap.data());
		},
		regname_vec,
		[fetcher](sdb::paddr_t pc){
			auto code=fetcher(pc);
			return sdb::vlen_inst_code(code.data,code.data+code.len);
		},
		disasm?[disasm](const sdb::disasmable_inst& inst){
			char buf[256];
			disasm(buf,sizeof(buf),
					inst.pc,(uint8_t*)inst.code.data(),inst.code.size());
			return string(buf);
		}:sdb::inst_disasmsembler(sdb::default_disasm)
	));
}

void sdb_destroy_debuger(sdb_debuger dbg){
	delete (sdb::debuger*)dbg;
}

#define _DBG (*((sdb::debuger*)dbg))

void sdb_enable_entrace(sdb_debuger dbg, int flags){
 _DBG.enable_difftest=(flags&SDB_ENTRACE_DIFFTEST);
 _DBG.enable_inst_trace=(flags&SDB_ENTRACE_INST);
 _DBG.enable_ftrace=(flags&SDB_ENTRACE_FTRACE);
 if(_DBG.enable_ftrace)assert(_DBG.enable_inst_trace);
 if(_DBG.enable_difftest)assert(_DBG.enable_inst_trace);
}

bool sdb_try_findload_elf_fromimg(sdb_debuger dbg, const char* img_file){
	return _DBG.try_findload_elf_fromimg(img_file);
}
void sdb_load_elf(sdb_debuger dbg, const char* filename){
	_DBG.load_elf(filename);
}
void sdb_load_difftest_ref(sdb_debuger dbg, const char* so_file, size_t img_size){
	_DBG.load_difftest_ref(so_file,img_size);
}
void sdb_exec(sdb_debuger dbg, const char* cmdline){
	_DBG.exec_command(cmdline);
}
void sdb_skip_difftest_ref(sdb_debuger dbg){
	_DBG.difftest_ref_skip();
}

void sdb_set_state(sdb_debuger dbg, sdbc_cpu_state state){
	sdb::cpu_state s;
	s.state=(sdb::run_state)state.state;
	s.pc=state.pc;
	s.halt_ret=state.halt_ret;
	_DBG.state()=s;
}
sdbc_cpu_state sdb_get_state(sdb_debuger dbg){
	sdbc_cpu_state state;
	auto s=_DBG.state();
	state.state=(sdb_run_state)s.state;
	state.pc=s.pc;
	state.halt_ret=s.halt_ret;
	return state;
}
