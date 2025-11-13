#include "sdb.hpp"
#include <assert.h>
#include "toks.hpp"

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


#define _ITEM(name,desc,f) {name,command_t(desc,this,&debuger::f)}

void debuger::_init_cmd_table(){

	_cmd_table={
		_ITEM("c", "Continue the execution of the program", resume),
		_ITEM("q", "Exit program",quit),
		/*
		_ITEM("si","Step the program for N instructions" ,
				size_t N=toks.empty();
				if(N||_parse(toks.front(), N))
					step(N);
				),
		_ITEM("info", "Display information about registers or watchpoints",
				auto type=toks.front();
				if(type=="r")dump_reg();
				else _error("unknown info command '{}'",type);
				),
		_ITEM("x", "Examine memory: x N EXPR",
				assert(ranges::distance(toks)==2);
				size_t N;
				bool good=true;
				good&=_parse(toks.front(),N);
				auto expr=*next(toks.begin());
				if(expr.starts_with("0x"))expr=expr.substr(2);
				paddr_t addr;
				good&=_parse(expr,addr, 16);
				if(good)dump_mem(addr, addr+N*4);
				)
				*/

		};
}

void debuger::exec_command(string_view cmdline){
	auto ec=exec(_cmd_table,cmdline);
	if(ec!=invoke_success){
		_error("exec failed: {}", ec);
	}
}
