package uart
import chisel3._
import chisel3.util._

import common_def._
import axi4._

import chisel3.util.circt.dpi._

class UARTUnit extends Module{
  val io = IO(AXI4IO.Slave)

  io.dontCareAR()
  io.dontCareR()

  val sio = io.slave

  sio.awready := true.B
  sio.wready  := true.B

  sio.bvalid := true.B
  sio.bresp  := AXI4IO.BResp.OKAY

  when(sio.wvalid){
    printf("%c", sio.wdata(7,0))
    RawClockedVoidFunctionCall("skip_difftest_ref")(
      clock,
      sio.wvalid
    )
  }
}
