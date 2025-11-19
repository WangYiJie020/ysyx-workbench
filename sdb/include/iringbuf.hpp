#pragma once
#include <sdb.hpp>
#include <disasm_trace.hpp>
namespace sdb {
	class iringbuf_trace_handler;
	using iringbuf_trace_handler_ptr=std::unique_ptr<iringbuf_trace_handler>;
	iringbuf_trace_handler_ptr make_iringbuf_trace_handler(inst_disasmsembler,size_t n_records=32);
}

