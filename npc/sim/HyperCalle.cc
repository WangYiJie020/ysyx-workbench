#include "../../abstract-machine/klib/include/hypercall.h"
#include "memory/mem.hpp"
#include "sdbWrap.hpp"
#include "sim.hpp"
#include <cstdint>

#define Ret(ret)                                                               \
  do {                                                                         \
    cpu.force_set_gpr(10, ret);                                                \
    sdb_skip_difftest_ref();                                                   \
    return;                                                                    \
  } while (0)

static std::errc do_hyperproc(__HyperCmd cmd, uint32_t a1, uint32_t a2,
                              uint32_t a3, uint32_t a4, uint32_t a5) {
  switch (cmd) {
  case __HCmd_memcpy: {
    uint32_t guest_dest = a1;
    uint32_t guest_src = a2;
    uint32_t size = a3;
    spdlog::debug("HyperCall memcpy: dest=0x{:08x} src=0x{:08x} size={}",
                  guest_dest, guest_src, size);

    // to support sdram which unsupport direct pointer access
    std::vector<uint8_t> srcvec;
    if (!read_guest_mem(guest_src, size, srcvec)) {
      spdlog::warn(
          "HyperCall memcpy failed to read guest memory src=0x{:08x} size={}",
          guest_src, size);
      return std::errc::io_error;
    }
    if (!write_guest_mem(guest_dest, srcvec)) {
      spdlog::warn(
          "HyperCall memcpy failed to write guest memory dest=0x{:08x} size={}",
          guest_dest, size);
      return std::errc::io_error;
    }

    sdb_memcpy_to_ref(guest_dest, srcvec);
    return std::errc{};
    break;
  }
  case __HCmd_memset: {
    uint32_t guest_dest = a1;
    uint8_t val = a2 & 0xff;
    uint32_t size = a3;
    spdlog::debug("HyperCall memset: dest=0x{:08x} val=0x{:02x} size={}",
                  guest_dest, val, size);

    std::vector<uint8_t> vec(size, val);
    if (!write_guest_mem(guest_dest, vec)) {
      spdlog::warn(
          "HyperCall memset failed to write guest memory dest=0x{:08x} size={}",
          guest_dest, size);
      return std::errc::io_error;
    }
    sdb_memcpy_to_ref(guest_dest, vec);
    return std::errc{};
  }
  default:
    break;
  };
  return std::errc::invalid_argument;
}

void check_do_hypercall() {
  auto &cfg = *sim_get_config();
  auto &cpu = *sim_get_cpu_state();
  if (cpu.pc != cfg.hypercall_addr)
    return;
  if (!cfg.setting.hypercall)
    return;

  spdlog::trace("HyperCall invoked at pc = 0x{:08x}", cpu.pc);
  __HyperCmd cmd = (__HyperCmd)cpu.gpr[10];
  uint32_t ret = (uint32_t)do_hyperproc(cmd, cpu.gpr[11], cpu.gpr[12],
                                        cpu.gpr[13], cpu.gpr[14], cpu.gpr[15]);
  Ret(ret);
}
