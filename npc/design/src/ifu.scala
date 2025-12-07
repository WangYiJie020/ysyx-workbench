package cpu
import chisel3._
import chisel3.util._
import common_def._
import busfsm._

import memory._
import axi4._

class IFU extends Module {
  val io = IO(new Bundle {
    val pc  = Flipped(Decoupled(Types.UWord))
    val mem = AXI4LiteIO.TX
    val out = Decoupled(new Inst)
  })

  val fsm = Module(new OneMasterOneSlaveFSM)
  fsm.connectMaster(io.pc)
  fsm.connectSlave(io.out)

  val lastPC = RegNext(io.pc.bits)
  val code = Reg(Types.UWord)
  val fetchDone = Reg(Bool())
  val pcChanged = io.pc.bits =/= lastPC

  dontTouch(code)
  dontTouch(fetchDone)
  dontTouch(io)

  io.mem.aw.valid := false.B
  io.mem.w.valid  := false.B

  io.mem.aw:= DontCare
  io.mem.w := DontCare
  io.mem.b := DontCare


  io.mem.ar.valid := io.pc.valid && (!fetchDone)
  io.mem.ar.bits  := io.pc.bits

  when(io.mem.ar.valid && io.mem.ar.ready) {
  }


  when(io.mem.r.valid && !fetchDone) {
    code := io.mem.r.bits.data
    fetchDone := true.B
  }
  io.mem.r.ready := true.B

  when(!fsm.io.master_valid || pcChanged) {
    fetchDone := false.B
  }

  fsm.io.self_finished := fetchDone

  // NOTICE: dpi function auto generated with void return
  // see https://github.com/llvm/circt/blob/main/docs/Dialects/FIRRTL/FIRRTLIntrinsics.md#dpi-intrinsic-abi
  io.out.bits.code := code
  io.out.bits.pc   := io.pc.bits
}


