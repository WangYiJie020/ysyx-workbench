#pragma once
#include <string>
#include <sdb.hpp>

namespace sdb {
	using inst_disasmsembler=
		std::function<std::string(paddr_t pc,vlen_inst_view)>;

	namespace _impl {
		std::string expand_tabs(std::string_view in, int tabsize=8);
	}

	std::string default_inst_disasm(paddr_t pc,vlen_inst_view inst);

	// disasm trace

	class disasm_trace_handler:public trace_handler{
		private:
			size_t _threshold;
			inst_disasmsembler _disasm;
		protected:
			std::string _dump_inst(
					_ctx_ref ctx, bool highlight_disasm=false
			);
		public:
			disasm_trace_handler(inst_disasmsembler d=default_inst_disasm,size_t th=16)
				:_threshold(th),_disasm(d){}
			virtual void handle(_ctx_ref ctx)override{
				_log("{}",_dump_inst(ctx));
			}
			virtual bool no_call_when_batch(size_t n)override{
				return n>_threshold;
			}
	};
	inline trace_handler_ptr make_disasm_trace_handler(
		inst_disasmsembler disasm=default_inst_disasm,
		size_t n_show_threshold=16
	){
		return std::make_shared<disasm_trace_handler>(disasm,n_show_threshold);
	}

	// iringbuf

	class iringbuf_trace_handler;
	trace_handler_ptr make_iringbuf_trace_handler(
			inst_disasmsembler=default_inst_disasm,size_t n_records=32
	);

	// ftrace

	class ftrace_handler;
	enum class jump_type{
		normal,
		call,
		ret
	};

	using jump_recognizer=std::function<jump_type(vlen_inst_view)>;
	jump_type default_riscv_jump_recognizer(vlen_inst_view inst);

	trace_handler_ptr make_ftrace_handler(
		std::string_view elf_file,
		jump_recognizer recog_jmp=default_riscv_jump_recognizer
	);

	// etrace
	
	enum class exception_type{
		none,
		ecall,
		eret
	};
	using exception_recognizer=std::function<exception_type(vlen_inst_view)>;
	exception_type default_riscv_exception_recognizer(vlen_inst_view inst);
	trace_handler_ptr make_etrace_handler(exception_recognizer recog_exc=default_riscv_exception_recognizer);

}

