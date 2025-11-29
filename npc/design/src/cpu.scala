package cpu

import chisel3._
import chisel3.util.{Decoupled, Enum}
import chisel3.util.MuxLookup

class Inst extends Bundle {
  val code = Output(UInt(32.W))
  val pc   = Output(UInt(32.W))
}

class IFU extends Module {
  val io                            = IO(new Bundle { val out = Decoupled(new Inst) })
  val s_idle :: s_wait_ready :: Nil = Enum(2)
  val state                         = RegInit(s_idle)
  state        := MuxLookup(state, s_idle)(
    List(
      s_idle       -> Mux(io.out.valid, s_wait_ready, s_idle),
      s_wait_ready -> Mux(io.out.ready, s_idle, s_wait_ready)
    )
  )
  io.out.valid := 0.B
}

object InstType extends ChiselEnum {
  val imm, reg, store, u, jump, branch = Value
}

class IDU extends Module {
  val io = IO(new Bundle {
    val in  = Flipped(Decoupled(new Inst))
    val out = Output(InstType())

  })
  // ...
}
