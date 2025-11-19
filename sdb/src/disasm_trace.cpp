#include <disasm_trace.hpp>

using namespace sdb;
using namespace std;

extern "C" void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte);
extern "C" void init_disasm();

string sdb::default_inst_disasm(paddr_t pc,vlen_inst_view inst){
	static bool has_init=false;
	if(!has_init){
		init_disasm();
		has_init=true;
	}
	char buf[256];
	disassemble(buf,sizeof(buf),pc,(uint8_t*)inst.data(),inst.size());
	return buf;
}

string sdb::_impl::expand_tabs(std::string_view in, int tabsize) {
    string out;
    out.reserve(in.size() * tabsize);
    int col = 0;
    for (char c : in) {
        if (c == '\t') {
            int spaces = tabsize - (col % tabsize);
            out.append(spaces, ' ');
            col += spaces;
        } else {
            out.push_back(c);
            col++;
        }
    }
    return out;
}

string sdb::disasm_trace_handler::_dump_inst(disasm_trace_handler::_ctx_ref ctx, bool highlight_disasm) {
	string res;
	res+=format(ANSI_FG_GRAY "0x{:08X}: {}{:25} " ANSI_FG_GRAY "(",
			ctx.pc,
			highlight_disasm?ANSI_FG_RED:ANSI_NONE,
			_disasm(ctx.pc,ctx.inst));
	for(size_t j=0;j<ctx.inst.size();j++){
		if(j) res+=format(" ");
	  res+=format("{:02X}",ctx.inst[j]);
	}
	auto as_u32code=*(uint32_t*)ctx.inst.data();
	res+=format(" `0x{:08X}",as_u32code);
	res+=format(")" ANSI_NONE "\n");
	return _impl::expand_tabs(res);
}
