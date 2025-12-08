package clint
import chisel3._
import chisel3.util._

import axi4._
import chisel3.util.circt.dpi.RawClockedVoidFunctionCall

class CLINTUnit extends Module {
  val io = IO(AXI4LiteIO.RX)

  io.aw := DontCare
  io.w  := DontCare
  io.b  := DontCare

  io.ar.ready := true.B
  io.r.valid  := true.B

  when(io.ar.valid){
    RawClockedVoidFunctionCall("skip_difftest_ref")(
      clock,
      io.ar.valid
    )
  }

  val mtime = RegInit(0.U(64.W))

  mtime := mtime + 1.U

  io.r.bits.resp := AXI4LiteIO.RResp.OKAY
  io.r.bits.data := Mux(io.ar.bits === 0x10000048.U, mtime(31, 0), Mux(io.ar.bits === 0x1000004c.U, mtime(63, 32), 0.U))
}
