#pragma once
#include <sdb.hpp>

namespace sdb {
	class ftrace_handler;
	using ftrace_handler_ptr=std::shared_ptr<ftrace_handler>;

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

};

