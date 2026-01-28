#include <iostream>
#include <string_view>
#include <verilated.h>
#include <verilated_vpi.h>

#include <spdlog/spdlog.h>

#include "sim.hpp"
#include "dbg.hpp"


int main(int argc, char **argv) {
  spdlog::set_level(spdlog::level::trace); // will modify all registered loggers

	auto& setting = sim_get_config()->setting;
	load_sim_setting_from_env(setting);

  if(!sim_init(argc, argv, setting)){
		get_dut()->final();
		spdlog::error("sim_init failed");
		return 1;
	}

	if(setting.gdb_mode){
	}else{
		sdb_mainloop();
	}

	spdlog::info("sim ended");
	
	get_dut()->final();
	return 0;
}
