#pragma once

#include <stdint.h>
#include <functional>
#include <format>
#include <iostream>
#include <ranges>
#include <vector>
#include <optional>

#include "cmd.hpp"

namespace sdb {

	using word_t=uint32_t;
	using sword_t=int32_t;

	using vaddr_t = word_t;
	using paddr_t = word_t;

using cpu_executor=std::function<void()>;
using addr_reader=std::function<uint8_t(paddr_t)>;
using reg_reader=std::function<std::optional<word_t>(std::string_view)>;

enum class run_state{
	running,
	stop,
	end,
	abort,
	quit
};

struct cpu_state{
	run_state state;
	vaddr_t halt_pc;
	uint32_t halt_ret;

	cpu_state():state(run_state::running),halt_pc(0),halt_ret(0){}
	cpu_state(run_state s,vaddr_t pc=0,uint32_t ret=0):
		state(s),halt_pc(pc),halt_ret(ret){}

	inline bool is_bad()const{
		bool good=
			(state==run_state::end&&halt_ret==0)
			||(state==run_state::quit);
		return !good;
	}
};
class debuger{
	cpu_executor _exec;
	cpu_state _state;

	addr_reader _paddr_read;
	reg_reader _reg_read;
	
	std::vector<std::string> _reg_names;
	using fmt_str=std::string_view;

	clscmd::command_table _cmd_table;

	inline void _print(fmt_str fmt, auto&&... args){
		std::cout<<vformat(fmt,std::make_format_args(args...));
	}
	inline void _error(fmt_str fmt, auto&&... args){
		std::cerr
			<<"Error: "
			<<vformat(fmt,std::make_format_args(args...))
			<<std::endl;
	}

	void _init_cmd_table();

	void cmd_info(std::string_view);
	void cmd_x(size_t N,paddr_t addr);

public:

	debuger(cpu_executor e,addr_reader mr,reg_reader rr,auto&& reg_names):
		_exec(e),_paddr_read(mr),_reg_read(rr),_reg_names(reg_names){
			_init_cmd_table();
		}

	const cpu_state& get_state()const{
		return _state;
	}

	inline void set_run_state(run_state s){
		_state.state=s;
	}
	inline bool is_running(){
		return _state.state==run_state::running;
	}	

	inline void resume()
	{
		step(-1);
	}
	void quit();
	inline void step(size_t n=1)
	{
		for(size_t i=0;i<n&&is_running();i++)_exec();
	}
	void dump_mem(vaddr_t addr,vaddr_t end);
	void dump_reg();

	void exec_command(std::string_view cmdline);
};


}


