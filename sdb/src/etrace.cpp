#include <tracers.hpp>
#include "ansi_col.h"

using namespace sdb;
using namespace std;

constexpr word_t INST_ECALL=0x00000073;
constexpr word_t INST_MRET=0x30200073;

exception_type sdb::default_riscv_exception_recognizer(vlen_inst_view inst){
	word_t code=*(word_t*)inst.data();
	if(code==INST_ECALL)return exception_type::ecall;
	if(code==INST_MRET)return exception_type::eret;
	return exception_type::none;
}

class etrace_handler:public trace_handler{
	private:
		exception_recognizer _recog_exc;
	public:
		etrace_handler(exception_recognizer r=default_riscv_exception_recognizer)
			:_recog_exc(r){}
		virtual void handle(_ctx_ref ctx)override{
			auto type=_recog_exc(ctx.inst);
			if(type==exception_type::none)return;
			string hint_str=type==exception_type::ecall?"ecall":"eret ";
			_log(
				"0x{:08X}: {}{}{}\n",
				ctx.pc,
				type==exception_type::ecall?ANSI_FG_YELLOW:ANSI_FG_BLUE,
				hint_str,
				ANSI_NONE
			);

		}
};

trace_handler_ptr sdb::make_etrace_handler(
	exception_recognizer recog_exc
){
	return make_shared<etrace_handler>(recog_exc);
}

