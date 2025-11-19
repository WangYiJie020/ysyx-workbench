#pragma once

#include <stdint.h>
#include <functional>
#include <format>
#include <iostream>
#include <vector>
#include <deque>
#include <memory>
#include <sstream>

#include "cmd.hpp"

namespace sdb {

	using word_t=uint32_t;
	using sword_t=int32_t;

	using paddr_t = uint32_t;

	using vlen_inst_code=std::vector<uint8_t>;
	using vlen_inst_view=std::span<const uint8_t>;

	using reg_snapshot_t=std::vector<word_t>;
	using reg_snapshot_view=std::span<const word_t>;

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

		inline bool is_badexit()const{
			bool good=
				(state==run_state::end&&halt_ret==0)
				||(state==run_state::quit);
			return !good;
		}

		void halt(uint32_t ret){
			state=run_state::end;
			halt_ret=ret;
		}
		void abort(){
			state=run_state::abort;
		}
		bool quited()const{
			return state==run_state::quit;
		}
	};

	struct trace_context{
		paddr_t pc;
		reg_snapshot_view regs;
		vlen_inst_view inst;
	};

	using output_iterator=std::ostream_iterator<char>;

	class trace_handler{
	private:
		std::ostringstream _logbuf;
		std::ostringstream _dmpbuf;

	protected:

		void _log(std::string_view fmt, auto&&... args){
			using namespace std;
			vformat_to(ostream_iterator<char>(_logbuf),
					fmt,make_format_args(args...));
		}
		void _dump(std::string_view fmt, auto&&... args){
			using namespace std;
			vformat_to(ostream_iterator<char>(_dmpbuf),
					fmt,make_format_args(args...));
		}
		void _error(std::string_view fmt, auto&&... args){
			_log("Error: ");
			_log(fmt,std::forward<decltype(args)>(args)...);
			_log("\n");
		}
		using _ctx_ref = const trace_context&;

	public:
		std::string get_log()const{return _logbuf.str();}
		std::string get_dump()const{return _dmpbuf.str();}

		virtual size_t max_call_percmd()const{return -1;}
		virtual void make_dump(){}
		virtual void handle(_ctx_ref)=0;
	};
	using trace_handler_ptr=std::shared_ptr<trace_handler>;

	struct expr_t{
		std::string_view raw;
		expr_t(){}
		expr_t(std::string_view s):raw(s){}
		uint64_t eval()const;
	};


	// impl should return npc(pc after exec)
	using cpu_executor=std::function<paddr_t()>;
	// impl should prepare n bytes continuously in mem corresponding to addr
	// and return the pointer to the first byte
	using mem_loader=std::function<uint8_t*(paddr_t addr,size_t n)>;
	// impl should fill reg_snapshot_t with current register values 
	//
	// passed reg_snapshot_t has more size than needed, which is reserved for pc
	// and future use. 
	// and those extra items are only set when difftest step, so their values
	// maybe garbage during normal use.
	//
	// !!!**NOTICE**!!!
	// never resize!
	// never modify/read items out of register range!
	using reg_snapshoter=std::function<void(reg_snapshot_t&)>;
	using inst_fetcher=std::function<vlen_inst_code(paddr_t pc)>;

namespace _impl {
#define _MAKE_DEF(name) \
		struct name##_imp; \
		struct _deleter_##name{\
			void operator()(name##_imp* ptr);\
		};\
		using name##_imptr=std::unique_ptr<name##_imp,_deleter_##name>;\

	_MAKE_DEF(difftest);
#undef _MAKE_DEF

}

class debuger{
	public:
		bool enable_difftest=false;
		bool enable_inst_trace=false;
private:

	using fmt_str=std::string_view;

	cpu_executor _exec;
	cpu_state _state;

	mem_loader _loadmem;

	reg_snapshoter _shot_reg;
	std::vector<std::string_view> _reg_names;
	reg_snapshot_t _reg_snap;

	inst_fetcher _fetch_inst;

	std::vector<trace_handler_ptr> _trace_handlers;

	const paddr_t _INITIAL_PC;

	constexpr static size_t _MAX_INST_DUMP_PERSTEP=10;
	bool _enable_dump_inst=true;

	_impl::difftest_imptr _imp_difftest=nullptr;

	clscmd::command_table _cmd_table;

	std::ostream_iterator<char> _o_iter{std::cout};

	inline void _print(fmt_str fmt, auto&&... args){
		vformat_to(_o_iter,fmt,std::make_format_args(args...));
	}
	inline void _error(fmt_str fmt, auto&&... args){
		std::cerr
			<<"Error: "
			<<vformat(fmt,std::make_format_args(args...))
			<<std::endl;
	}

	void _init_cmd_table();
	void _difftest_step(paddr_t pc,paddr_t npc);
	void _step_one();

	void _dump_iringbuf();

	void cmd_q();
	void cmd_info(std::string_view);
	void cmd_x(size_t N,expr_t addr);

	inline void cmd_c(){cmd_si(-1);}
	inline void cmd_si(size_t n=1){
		if(_state.state==run_state::end||_state.state==run_state::quit){
			_error("Program has ended. Cannot execuate.");
			return;
		}
		_enable_dump_inst=n<=_MAX_INST_DUMP_PERSTEP;
		for(;n>0&&is_running();n--)_step_one();
	}

	void dump_mem(paddr_t addr,paddr_t end);
	void dump_reg();


public:

	debuger(
			paddr_t init_pc,
			cpu_executor e,mem_loader ml,reg_snapshoter rss,auto&& regnames,
			inst_fetcher f=inst_fetcher()
	): _exec(e),_loadmem(ml),_shot_reg(rss),_reg_names(regnames),
	_fetch_inst(f),_INITIAL_PC(init_pc){
		_state.pc=init_pc;
		_reg_snap.resize(_reg_names.size()+1);
		_init_cmd_table();
	}

	void add_trace(trace_handler_ptr th){
		_trace_handlers.push_back(th);
	}

	void load_difftest_ref(std::string_view so_file,size_t img_size,int port);

	void difftest_ref_skip();

	inline const cpu_state& state()const{return _state;}
	inline cpu_state& state(){return _state;}

	inline bool is_running(){
		return _state.state==run_state::running;
	}	
	inline void abort(){
		_state.abort();
		_dump_iringbuf();
	}
	void exec_command(std::string_view cmdline);
};

}
