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

  val sio = io.slave

  sio.arready := true.B
  sio.rvalid  := true.B

  when(sio.arvalid){
    RawClockedVoidFunctionCall("skip_difftest_ref")(
      clock,
      sio.arvalid
    )
  }

  val mtime = RegInit(0.U(64.W))

  mtime := mtime + 1.U

  sio.rresp := AXI4IO.RResp.OKAY
  sio.rdata := Mux(sio.araddr === 0x10000048.U, mtime(31, 0), Mux(sio.araddr === 0x1000004c.U, mtime(63, 32), 0.U))
}
