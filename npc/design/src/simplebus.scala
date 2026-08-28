package simplebus

import chisel3._

/** E-stage SimpleBus: one outstanding request, response valid for one cycle. */
class SimpleBusIMEM extends Bundle {
  val reqValid  = Output(Bool())
  val addr      = Output(UInt(32.W))
  val respValid = Input(Bool())
  val rdata     = Input(UInt(32.W))
}

class SimpleBusDMEM extends Bundle {
  val reqValid  = Output(Bool())
  val addr      = Output(UInt(32.W))
  val size      = Output(UInt(2.W))
  val wen       = Output(Bool())
  val wdata     = Output(UInt(32.W))
  val wmask     = Output(UInt(4.W))
  val respValid = Input(Bool())
  val rdata     = Input(UInt(32.W))
}
