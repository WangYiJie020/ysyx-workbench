extern "C" {
#include <gdbstub.h>
}
#include <stdint.h>

static size_t _op_get_regbytes(int regno) { return 4; }

static int _op_read_reg(void *args, int regno, void *value) {
  *(uint32_t *)value = 0x12345678;
  return 0;
}

static int _op_write_reg(void *args, int regno, void *value) { return 0; }

static int _op_read_mem(void *args, size_t addr, size_t len, void *val) {
  return 0;
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
#define MY_TARGET_RV32                                                         \
  "<?xml version=\"1.0\"?>"                                                    \
  "<!DOCTYPE target SYSTEM \"gdb-target.dtd\">"                                \
  "<target>"                                                                   \
  "  <architecture>riscv</architecture>"                                       \
  "  <feature name=\"org.gnu.gdb.riscv.cpu\">"                                 \
  "    <reg name=\"pc\" bitsize=\"32\" type=\"code_ptr\"/>"                    \
  "    <reg name=\"ra\" bitsize=\"32\" type=\"data_ptr\"/>"                    \
  "    <reg name=\"sp\" bitsize=\"32\" type=\"data_ptr\"/>"                    \
  "    <reg name=\"gp\" bitsize=\"32\" type=\"data_ptr\"/>"                    \
  "    <reg name=\"tp\" bitsize=\"32\" type=\"data_ptr\"/>"                    \
  "    <reg name=\"t0\" bitsize=\"32\"/>"                                      \
  "    <reg name=\"t1\" bitsize=\"32\"/>"                                      \
  "    <reg name=\"t2\" bitsize=\"32\"/>"                                      \
  "    <reg name=\"s0\" bitsize=\"32\"/>"                                      \
  "    <reg name=\"s1\" bitsize=\"32\"/>"                                      \
  "    <reg name=\"a0\" bitsize=\"32\"/>"                                      \
  "    <reg name=\"a1\" bitsize=\"32\"/>"                                      \
  "    <reg name=\"a2\" bitsize=\"32\"/>"                                      \
  "    <reg name=\"a3\" bitsize=\"32\"/>"                                      \
  "    <reg name=\"a4\" bitsize=\"32\"/>"                                      \
  "    <reg name=\"a5\" bitsize=\"32\"/>"                                      \
  "    <reg name=\"a6\" bitsize=\"32\"/>"                                      \
  "    <reg name=\"a7\" bitsize=\"32\"/>"                                      \
  "    <reg name=\"s2\" bitsize=\"32\"/>"                                      \
  "    <reg name=\"s3\" bitsize=\"32\"/>"                                      \
  "    <reg name=\"s4\" bitsize=\"32\"/>"                                      \
  "    <reg name=\"s5\" bitsize=\"32\"/>"                                      \
  "    <reg name=\"s6\" bitsize=\"32\"/>"                                      \
  "    <reg name=\"s7\" bitsize=\"32\"/>"                                      \
  "    <reg name=\"s8\" bitsize=\"32\"/>"                                      \
  "    <reg name=\"s9\" bitsize=\"32\"/>"                                      \
  "    <reg name=\"s10\" bitsize=\"32\"/>"                                     \
  "    <reg name=\"s11\" bitsize=\"32\"/>"                                     \
  "    <reg name=\"t3\" bitsize=\"32\"/>"                                      \
  "    <reg name=\"t4\" bitsize=\"32\"/>"                                      \
  "    <reg name=\"t5\" bitsize=\"32\"/>"                                      \
  "    <reg name=\"t6\" bitsize=\"32\"/>"                                      \
  "  </feature>"                                                               \
  "</target>"
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
      .target_desc = (char *)MY_TARGET_RV32,
      .smp = 0,
      .reg_num = 32,
  };

  return gdbstub_init(&gdbstub, &ops, arch, (char *)socket);
}
bool gdbop_run() { return gdbstub_run(&gdbstub, nullptr); }
void gdbop_close() { gdbstub_close(&gdbstub); }
