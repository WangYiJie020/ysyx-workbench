package uart
import chisel3._
import chisel3.util._

import common_def._
import axi4._

class UARTUnit extends Module{
  val io = IO(AXI4LiteIO.RX)

  io.ar:= DontCare
  io.r := DontCare

  io.aw.ready := true.B
  io.w.ready  := true.B

  io.b.valid := true.B
  io.b.bits  := AXI4LiteIO.BResp.OKAY

  when(io.w.valid){
    printf("%c", io.w.bits.data(7,0))
  }
}
