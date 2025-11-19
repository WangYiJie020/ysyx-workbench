#pragma once
#include <sdb.hpp>
#include <disasm_trace.hpp>
namespace sdb {
	class iringbuf_trace_handler;
	trace_handler_ptr make_iringbuf_trace_handler(
			inst_disasmsembler=default_inst_disasm,size_t n_records=32
	);
}

