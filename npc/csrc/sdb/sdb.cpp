#include "sdb.hpp"
#include <assert.h>

using namespace std;
using namespace sdb;
using namespace std::views;
using namespace clscmd;

template <typename T>
struct std::formatter<optional<T>> : formatter<T> {
    auto format(const optional<T>& opt, auto& ctx) const {
        if (opt)
            return formatter<T>::format(*opt, ctx);
        else
            return format_to(ctx.out(), "(nullopt)");
    }
};

template<>
struct std::formatter<errc> : formatter<std::string> {
    auto format(errc ec, auto& ctx) const {
        return formatter<std::string>::format(
            std::make_error_code(ec).message(), ctx);
    }
};

static string expand_tabs(std::string_view in, int tabsize) {
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

void debuger::_step_one(){
	if constexpr (_ENABLE_ITRACE){
		_print("{}\n", _disasm(_state.pc));
	}
	_state.pc = _exec();	
}

uint64_t expr_t::eval()const{
	// only support 0x... now
	auto s=raw;
	assert(s.starts_with("0x"));
	s=s.substr(2);
	uint64_t v;
	auto ec=_impl::parse(s,v);
	if(ec!=errc())cerr<<format("failed to parse {} : {}",s,ec)<<endl;
	return v;
}

void debuger::quit(){
	if(is_running()||!_state.is_bad()){
		_state.state=run_state::quit;
	}
}

void debuger::dump_mem(paddr_t addr,paddr_t end){
	assert((end-addr)%4==0);
	while (addr!=end) {
		_print("0x{:08x}: ",addr);	
		for(int i=0;i<4;i++)
			_print("{:02x} ", _paddr_read(addr+i));
		_print("\n");
		addr+=4;
	}
}
void debuger::dump_reg(){
	for(auto r:_reg_names){
		auto v=_reg_read(r);
		_print("{}:\t0x{:08x}\t{}\n", r, v,v);
	}
}

void debuger::cmd_info(string_view s){
	if(s=="r")dump_reg();
	else return _error("Unknown info command {}", s);	
}
void debuger::cmd_x(size_t N,expr_t e_addr){
	paddr_t addr=e_addr.eval();
	_print("call x {} {:08x}", N,addr);
	cout<<endl;
	dump_mem(addr, addr+N*4);
}

#define _ITEM(name,desc,...) {name,command_t(desc,this,&debuger::__VA_ARGS__)}

void debuger::_init_cmd_table(){
	_cmd_table={
		_ITEM("c", "Continue the execution of the program", resume),
		_ITEM("q", "Exit program",quit),
		_ITEM("si","Step the program for N instructions",step,1),
		_ITEM("info", "Display information about registers or watchpoints",cmd_info),
		_ITEM("x", "Examine memory: x N EXPR",cmd_x),
		};
}

void debuger::exec_command(string_view cmdline){
	auto ec=exec(_cmd_table,cmdline);
	if(ec!=invoke_success){
		_error("{}", ec);
	}
}
