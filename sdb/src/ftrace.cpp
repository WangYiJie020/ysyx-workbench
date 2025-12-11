#include <tracers.hpp>
#include "ansi_col.h"
#include "elf_tool.hpp"

using namespace std;
using namespace sdb;

jump_type sdb::default_riscv_jump_recognizer(vlen_inst_view inst){
	word_t instr=*(word_t*)inst.data();
	uint8_t opcode=instr&0x7f;
	uint8_t funct3=(instr>>12)&0x7;
	uint8_t rd=(instr>>7)&0x1f;
	uint8_t rs1=(instr>>15)&0x1f;
	bool is_jal=opcode==0x6f;
	bool is_jalr=opcode==0x67 && funct3==0x0;
	bool is_ret=is_jalr && rd==0 && rs1==1;
	bool is_call=(is_jal && rd==1) || (is_jalr && rd==1);
	if(is_call)return jump_type::call;
	if(is_ret)return jump_type::ret;
	return jump_type::normal;
}
class sdb::ftrace_handler:public trace_handler{
	private:
		elf_handler elf;
		jump_recognizer recog_jmp;
		int func_depth=0;
	public:
		ftrace_handler(
			string_view elf_file,
			jump_recognizer r
		):recog_jmp(r),func_depth(0){
			fstream fs(string(elf_file),ios::in|ios::binary);
			elf.load(fs);
			_log("Loaded ELF file {}\n",elf_file);
		}
		virtual void handle(_ctx_ref ctx)override{
			auto type=recog_jmp(ctx.inst);
			if(type==jump_type::normal)return;
			auto hint_str=type==jump_type::call?"call fun":"ret from";
			if(type==jump_type::call)func_depth++;

			auto f=elf.get_fun_at(ctx.lastpc);
			auto fname=f?f->name:"(unknown)";

			_log(
					"0x{:08X}: "
					"{}{} "
					ANSI_FG_GRAY "f`{:08X}"
					ANSI_NONE "{}{}\n",
				ctx.pc,
				type==jump_type::call?ANSI_FG_YELLOW:ANSI_FG_BLUE,
				hint_str,
				f?f->addr:0,
				string(func_depth,' '),
				fname
			);

			if(type==jump_type::ret){
				if(func_depth>0)func_depth--;
				else _error("ret but func depth is 0");
			}

		}
};

trace_handler_ptr sdb::make_ftrace_handler(
	string_view elf_file,
	jump_recognizer recog_jmp
){
	return std::make_shared<ftrace_handler>(elf_file,recog_jmp);
}

