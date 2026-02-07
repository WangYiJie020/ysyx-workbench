package clint
import chisel3._
import chisel3.util._

import axi4._
import chisel3.util.circt.dpi.RawClockedVoidFunctionCall

class CLINTUnit extends Module {
  val io = IO(AXI4IO.Slave)

  io.dontCareAW()
  io.dontCareW()
  io.dontCareB()

  val sio = io

  io.dontCareNonLiteB()
  io.dontCareNonLiteR()

  sio.arready := true.B
  sio.rvalid  := sio.arvalid
  assert(!(sio.arvalid && !sio.rready), "CLINTUnit does not support wait readdata")

  when(sio.arvalid){
    RawClockedVoidFunctionCall("skip_difftest_ref")(
      clock,
      sio.arvalid
    )
  }

  val mtime = RegInit(0.U(64.W))

  mtime := mtime + 1.U

  sio.rresp := AXI4IO.RResp.OKAY
  // h02000000-h0200ffff
  // sio.rdata := Mux(sio.araddr === 0x02000048.U, mtime(31, 0), Mux(sio.araddr === 0x0200004c.U, mtime(63, 32), 0.U))
  // val araddrLow = sio.araddr(3, 0)
  // 0x48 -> 8 -> 1000
  // 0x4c -> 12 -> 1100
  val isRdHi = sio.araddr(2)
  sio.rdata := Mux(isRdHi, mtime(63, 32), mtime(31, 0))
}
