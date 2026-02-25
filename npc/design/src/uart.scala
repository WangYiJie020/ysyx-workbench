package uart
import chisel3._
import chisel3.util._

import common_def._
import axi4._

import chisel3.util.circt.dpi._

import chisel3.layer._
object InlinePrintfLayer extends Layer(LayerConfig.Inline)

object InlinePrintf {
  def apply(pable: Printable) = {
    layer.block(InlinePrintfLayer) {
      printf(pable)
    }
  }
}

class UARTUnit extends Module {
  val io = IO(AXI4IO.Slave)

  io.dontCareAR()
  io.dontCareR()

  io.dontCareNonLiteB()

  val sio = io

  sio.awready := true.B
  sio.wready  := true.B

  sio.bvalid := true.B
  sio.bresp  := AXI4IO.BResp.OKAY

  when(sio.wvalid) {
    val chData = sio.wdata(7, 0)
    InlinePrintf(cf"$chData%c")
    RawClockedVoidFunctionCall("skip_difftest_ref")(
      clock,
      sio.wvalid
    )
  }
}
