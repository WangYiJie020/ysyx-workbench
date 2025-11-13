#pragma once

#include <stdint.h>
#include <functional>
#include <format>
#include <iostream>
#include <vector>
#include <optional>

#include "cmd.hpp"

namespace sdb {

	using word_t=uint32_t;
	using sword_t=int32_t;

	// phiysical addr
	using paddr_t = word_t;

// should return pc after exec
using cpu_executor=std::function<paddr_t()>;
using addr_reader=std::function<uint8_t(paddr_t)>;
using reg_reader=std::function<std::optional<word_t>(std::string_view)>;
using inst_disasmsembler=std::function<std::string(uint64_t pc)>;

enum class run_state{
	running,
	stop,
	end,
	abort,
	quit
};

struct cpu_state{
	run_state state;
	paddr_t pc;
	uint32_t halt_ret;

	cpu_state():state(run_state::running),pc(0),halt_ret(0){}
	cpu_state(run_state s,paddr_t pc=0,uint32_t ret=0):
		state(s),pc(pc),halt_ret(ret){}

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

	inst_disasmsembler _disasm;
	constexpr static bool _ENABLE_ITRACE=false;
	
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

	void _step_one();

	void cmd_info(std::string_view);
	void cmd_x(size_t N,clscmd::expr_t addr);

public:

	debuger(
			paddr_t init_pc,
			cpu_executor e,addr_reader mr,reg_reader rr,auto&& reg_names,
			inst_disasmsembler d=inst_disasmsembler()
			):
		_exec(e),_paddr_read(mr),_reg_read(rr),_disasm(d),_reg_names(reg_names){
			_state.pc=init_pc;
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
		for(size_t i=0;i<n&&is_running();i++)_step_one();
	}
	void dump_mem(paddr_t addr,paddr_t end);
	void dump_reg();

	void exec_command(std::string_view cmdline);
};


}


