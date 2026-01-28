extern "C" {
#include <gdbstub.h>
}
#include <stdint.h>

#include "common.hpp"

#include "sim.hpp"

static size_t _op_get_regbytes(int regno) { return 4; }

static int _op_read_reg(void *args, int regno, void *value) {
	return sim_get_cpu_state()->gpr[regno];
}

static int _op_write_reg(void *args, int regno, void *value) {
	sim_get_cpu_state()->gpr[regno] = *(uint32_t *)value;
	return 0;
}

static int _op_read_mem(void *args, size_t addr, size_t len, void *val) {

  
}

static int _op_write_mem(void *args, size_t addr, size_t len, void *val) {
  return 0;
}

static bool _op_set_bp(void *args, size_t addr, bp_type_t type) { return true; }

static bool _op_del_bp(void *args, size_t addr, bp_type_t type) { return true; }

static void _op_on_interrupt(void *args) {}

static void _op_set_cpu(void *args, int cpuid) {}
static int _op_get_cpu(void *args) { return 0; }

static gdb_action_t _op_cont(void *args) { return ACT_RESUME; }
static gdb_action_t _op_stepi(void *args) { return ACT_RESUME; }

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
      .smp = 0,
      .reg_num = 32,
  };

  return gdbstub_init(&gdbstub, &ops, arch, (char *)socket);
}
bool gdbop_run() { return gdbstub_run(&gdbstub, nullptr); }
void gdbop_close() { gdbstub_close(&gdbstub); }


int gdb_mainloop() {
  spdlog::info("sim started in gdb debug mode");
  constexpr std::string_view gdb_socket = "127.0.0.1:1234";
  spdlog::info("initializing gdbstub at {}", gdb_socket);
  spdlog::info("this step will stuck until gdb connects");
  spdlog::info("try use 'target remote {}' in gdb", gdb_socket);

  bool res = gdbop_init(gdb_socket.data());
  if (!res) {
    spdlog::error("gdbop_init failed");
    return 1;
  }
  spdlog::info("gdbstub initialized, waiting for gdb commands");
  res = gdbop_run();
  if (!res) {
    spdlog::error("gdbop_run failed");
    return 1;
  }
  gdbop_close();
  spdlog::info("gdb session ended");
  return 0;
}
