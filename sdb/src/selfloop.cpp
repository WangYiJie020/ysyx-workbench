#include <cstdint>
#include <sdb.hpp>
#include <tracers.hpp>

void sdb::self_loop_trace_handler::handle(_ctx_ref ctx) {
  uint32_t inst = *(uint32_t *)ctx.inst.data();
  if (inst == 0x0000006f) { // riscv jal x0,0
    // self loop detected, stop the cpu
    _log("deadloop (jump to self) detected at pc={:#x}, stop the cpu\n",
         ctx.pc);
    name = "self-loop-detector";
    _req_stop();
  }
}
