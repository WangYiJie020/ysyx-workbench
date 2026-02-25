package uart
import chisel3._
import chisel3.util._

import common_def._
import axi4._

import chisel3.util.circt.dpi._

import chisel3.layer._
object InlineLayer extends Layer(LayerConfig.Inline) {
  override def name: String = "ysyx_25100261_InlineLayer"
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
    layer.block(InlineLayer) {
      printf(cf"$chData%c")
    }
    RawClockedVoidFunctionCall("skip_difftest_ref")(
      clock,
      sio.wvalid
    )
  }
}
