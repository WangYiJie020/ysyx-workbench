package uart
import chisel3._
import chisel3.util._

import common_def._
import axi4._

class UARTUnit extends Module{
  val io = IO(AXI4LiteIO.RX)
}
