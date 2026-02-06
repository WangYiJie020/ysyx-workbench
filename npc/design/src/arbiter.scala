package cpu

import chisel3._
import axi4._

// make exu and ifu access memory
class EXUIFU_MemVisitArbiter extends Module {
  val io = IO(new Bundle {
    val exu = AXI4IO.Slave
    val ifu = AXI4IO.Slave

    val out = AXI4IO.Master
  })

  // Simple arbiter, since IFU and EXU won't access memory at the same time

  val isExuReg = Reg(Bool())
  val isIfuReg = Reg(Bool())

  val ifuIO = io.ifu
  val exuIO = io.exu
  val outIO = io.out

  val isExu = (isExuReg && (!ifuIO.arvalid)) || (exuIO.arvalid)
  val isIfu = (isIfuReg && (!exuIO.arvalid)) || (ifuIO.arvalid)

  when(exuIO.arvalid) {
    isExuReg := true.B
    isIfuReg := false.B
  }.elsewhen(ifuIO.arvalid) {
    isExuReg := false.B
    isIfuReg := true.B
  }

  // AR channel
  outIO.arvalid := exuIO.arvalid || ifuIO.arvalid
  outIO.araddr  := Mux(isExu, exuIO.araddr, ifuIO.araddr)

  outIO.arid    := Mux(isExu, exuIO.arid, ifuIO.arid)
  outIO.arlen   := Mux(isExu, exuIO.arlen, ifuIO.arlen)
  outIO.arsize  := Mux(isExu, exuIO.arsize, ifuIO.arsize)
  outIO.arburst := Mux(isExu, exuIO.arburst, ifuIO.arburst)

  exuIO.arready := isExu && outIO.arready
  ifuIO.arready := isIfu && outIO.arready

  // R channel
  AXI4IO.noShakeConnectR(exuIO, outIO)
  AXI4IO.noShakeConnectR(ifuIO, outIO)

  exuIO.rvalid := isExu && outIO.rvalid
  ifuIO.rvalid := isIfu && outIO.rvalid

  outIO.rready := Mux(isExu, exuIO.rready, ifuIO.rready)

  // AW, W, B channel
  //   since only exu need write
  AXI4IO.connectAW(exuIO, outIO)
  AXI4IO.connectW(exuIO, outIO)
  AXI4IO.connectB(exuIO, outIO)

  io.ifu.dontCareAW()
  io.ifu.dontCareW()
  io.ifu.dontCareB()
}
