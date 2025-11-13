#include "sdb.hpp"
#include <assert.h>

using namespace std;
using namespace sdb;
using namespace std::views;
using namespace clscmd;

template <typename T>
struct std::formatter<std::optional<T>> : std::formatter<T> {
    auto format(const std::optional<T>& opt, auto& ctx) const {
        if (opt)
            return std::formatter<T>::format(*opt, ctx);
        else
            return std::format_to(ctx.out(), "(nullopt)");
    }
};

template<>
struct std::formatter<std::errc> : std::formatter<std::string> {
    auto format(std::errc ec, auto& ctx) const {
        return std::formatter<std::string>::format(
            std::make_error_code(ec).message(), ctx);
    }
};

void debuger::quit(){
	if(is_running()||!_state.is_bad()){
		_state.state=run_state::quit;
	}
}

void debuger::dump_mem(vaddr_t addr,vaddr_t end){
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
void debuger::cmd_x(size_t N,paddr_t addr){
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
