#pragma once

#include <string>
#include <sdb.hpp>
#include <ansi_col.h>

namespace sdb {
	using inst_disasmsembler=
		std::function<std::string(paddr_t pc,vlen_inst_view)>;

	namespace _impl {
		std::string expand_tabs(std::string_view in, int tabsize);
	}

	std::string default_inst_disasm(paddr_t pc,vlen_inst_view inst);

	class disasm_trace_handler:public trace_handler{
		private:
			inst_disasmsembler _disasm;
		protected:
			void _dump_inst(
					_ctx_ref ctx, bool highlight_disasm=false
			);
		public:
			disasm_trace_handler(inst_disasmsembler d=default_inst_disasm):_disasm(d){}
			virtual void handle(_ctx_ref ctx)override{
				_dump_inst(ctx);
				_log(get_dump());
			}
	};
	inline trace_handler_ptr make_disasm_trace_handler(
		inst_disasmsembler disasm=default_inst_disasm
	){
		return std::make_shared<disasm_trace_handler>(disasm);
	}

}
