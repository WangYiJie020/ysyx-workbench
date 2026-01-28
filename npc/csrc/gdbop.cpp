#include <cstdint>
#include <system_error>
extern "C" {
#include <gdbstub.h>
}
#include <stdint.h>

#include "common.hpp"
#include <spdlog/sinks/stdout_color_sinks.h>

#include "sim.hpp"

static std::shared_ptr<spdlog::logger> _logger =
    spdlog::stdout_color_mt("gdbstub");

static size_t _op_get_regbytes(int regno) { return 4; }

static int _op_read_reg(void *args, int regno, void *value) {
  if (regno < 0 || regno > 32) {
    _logger->error("gdb read reg invalid regno {}", regno);
    return (int)std::errc::invalid_argument;
  } else if (regno == 32) {
    // pc
    uint32_t pc = sim_get_cpu_state()->pc;
    _logger->trace("gdb read reg pc value {:08x}", pc);
    *(uint32_t *)value = pc;
    return 0;
  } else {
    uint32_t r = sim_get_cpu_state()->gpr[regno];
    // _logger->trace("gdb read reg {} value {:08x}", regno, r);
    *(uint32_t *)value = r;
    return 0;
  }
}

static int _op_write_reg(void *args, int regno, void *value) {
	if (regno < 0 || regno > 32) {
		_logger->error("gdb write reg invalid regno {}", regno);
		return (int)std::errc::invalid_argument;
	} else if (regno == 32) {
		// pc
		uint32_t pc = *(uint32_t *)value;
		_logger->trace("gdb write reg pc value {:08x}", pc);
		sim_get_cpu_state()->pc = pc;
		return 0;
	} else {
		uint32_t r = *(uint32_t *)value;
		_logger->trace("gdb write reg {} value {:08x}", regno, r);
		sim_get_cpu_state()->gpr[regno] = r;
		return 0;
	}
  return 0;
}

static int _op_read_mem(void *args, size_t addr, size_t len, void *val) {
  size_t end = addr + len;
  uint32_t *p = (uint32_t *)val;
  for (size_t offset = 0; offset < len; offset += 4) {
    uint32_t data = 0;
    bool ok = sim_read_vmem(addr + offset, &data);
    if (!ok) {
      return (int)std::errc::address_family_not_supported;
    }
    *p = data;
    p++;
  }
  return 0;
}

static int _op_write_mem(void *args, size_t addr, size_t len, void *val) {
  size_t end = addr + len;
  uint32_t *p = (uint32_t *)val;
  for (size_t offset = 0; offset < len; offset += 4) {
    bool ok = sim_write_vmem(addr + offset, *p);
    if (!ok) {
      return (int)std::errc::address_family_not_supported;
    }
    p++;
  }
  return 0;
}

static std::set<size_t> _breakpoints;

static bool _op_set_bp(void *args, size_t addr, bp_type_t type) {
  _logger->trace("gdb set bp at addr {:08x}", addr);
	_breakpoints.insert(addr);
	_logger->trace("current breakpoints count {}", _breakpoints.size());
  return true;
}

static bool _op_del_bp(void *args, size_t addr, bp_type_t type) {
  _logger->trace("gdb del bp at addr {:08x}", addr);
	_breakpoints.erase(addr);
	_logger->trace("current breakpoints count {}", _breakpoints.size());
  return true;
}

static void _op_on_interrupt(void *args) {}

static void _op_set_cpu(void *args, int cpuid) {}
static int _op_get_cpu(void *args) { return 0; }

static void _cb_on_halt(int a0) {
	_logger->info("sim halted callback called with a0={}", a0);
}

static gdb_action_t _op_cont(void *args) {
  _logger->trace("gdb cont called");
	while (true) {
		uint32_t pc = sim_get_cpu_state()->pc;
		if (_breakpoints.count(pc)) {
			_logger->info("hit breakpoint at pc {:08x}", pc);
			break;
		}
		sim_step_inst();
		if (sim_halted()) {
			_logger->info("sim halted at pc {:08x}", sim_get_cpu_state()->pc);
			return ACT_SHUTDOWN;
			break;
		}
	}
  return ACT_RESUME;
}
static gdb_action_t _op_stepi(void *args) {
  _logger->trace("gdb stepi called");
  sim_step_inst();
  return ACT_RESUME;
}

static gdbstub_t gdbstub;

bool gdbop_init(const char *socket) {
  static struct target_ops ops = {
      .cont = _op_cont,
      .stepi = _op_stepi,
      .get_reg_bytes = _op_get_regbytes,
      .read_reg = _op_read_reg,
      .write_reg = _op_write_reg,
      .read_mem = _op_read_mem,
      .write_mem = _op_write_mem,
      .set_bp = _op_set_bp,
      .del_bp = _op_del_bp,
      .on_interrupt = _op_on_interrupt,
      .set_cpu = _op_set_cpu,
      .get_cpu = _op_get_cpu,
  };

  static arch_info_t arch = {
      .target_desc = (char *)TARGET_RV32,
      .smp = 1,
      .reg_num = 32,
  };

  return gdbstub_init(&gdbstub, &ops, arch, (char *)socket);
}
bool gdbop_run() { return gdbstub_run(&gdbstub, nullptr); }
void gdbop_close() { gdbstub_close(&gdbstub); }

int gdb_mainloop() {
  spdlog::info("sim started in gdb debug mode");

	auto &cfg = *sim_get_config();
	cfg.raise_halt_cb = _cb_on_halt;

  // for (int i = 0; i < 4; i++) {
  //   _logger->trace("run preload step {}", i);
  //   sim_step_inst();
  // }
  // _logger->info("preload steps done, pc={:08x}", sim_get_cpu_state()->pc);

  constexpr std::string_view gdb_socket = "127.0.0.1:1234";
  _logger->info("initializing gdbstub at {}", gdb_socket);
  _logger->info("this step will stuck until gdb connects");
  _logger->info("try use 'target remote {}' in gdb", gdb_socket);

  bool res = gdbop_init(gdb_socket.data());
  if (!res) {
    _logger->error("gdbop_init failed");
    return 1;
  }
  spdlog::info("gdbstub initialized, waiting for gdb commands");
  res = gdbop_run();
  if (!res) {
    _logger->error("gdbop_run failed");
    return 1;
  }
  gdbop_close();
  spdlog::info("gdb session ended");
  return 0;
}
