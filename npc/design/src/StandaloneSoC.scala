package standalone

import chisel3._
import chisel3.util.HasBlackBoxInline
import config.CpuConfig
import top.ysyx_25100261

class StandaloneMemory extends BlackBox with HasBlackBoxInline {
  val io = IO(new Bundle {
    val clock = Input(Clock())
    val reset = Input(Bool())
    val ifuReqValid = Input(Bool())
    val ifuAddr = Input(UInt(32.W))
    val ifuRespValid = Output(Bool())
    val ifuRdata = Output(UInt(32.W))
    val lsuReqValid = Input(Bool())
    val lsuAddr = Input(UInt(32.W))
    val lsuWen = Input(Bool())
    val lsuWdata = Input(UInt(32.W))
    val lsuWmask = Input(UInt(4.W))
    val lsuRespValid = Output(Bool())
    val lsuRdata = Output(UInt(32.W))
  })

  setInline(
    "StandaloneMemory.sv",
    """module StandaloneMemory(
      |  input clock, input reset,
      |  input ifuReqValid, input [31:0] ifuAddr,
      |  output reg ifuRespValid, output reg [31:0] ifuRdata,
      |  input lsuReqValid, input [31:0] lsuAddr, input lsuWen,
      |  input [31:0] lsuWdata, input [3:0] lsuWmask,
      |  output reg lsuRespValid, output reg [31:0] lsuRdata
      |);
      |  import "DPI-C" function int pmem_read(input int addr);
      |  import "DPI-C" function void pmem_write(input int addr, input int data, input byte mask);
      |  always @(posedge clock) begin
      |    if (reset) begin
      |      ifuRespValid <= 1'b0;
      |      lsuRespValid <= 1'b0;
      |    end else begin
      |      ifuRespValid <= ifuReqValid;
      |      lsuRespValid <= lsuReqValid;
      |      if (ifuReqValid) ifuRdata <= pmem_read(ifuAddr);
      |      if (lsuReqValid && lsuWen) pmem_write(lsuAddr, lsuWdata, {4'b0, lsuWmask});
      |      if (lsuReqValid && !lsuWen) lsuRdata <= pmem_read(lsuAddr);
      |    end
      |  end
      |endmodule
      |""".stripMargin
  )
}

class TestSoC(config: CpuConfig) extends Module {
  val cpu = Module(new ysyx_25100261(config))
  val memory = Module(new StandaloneMemory)

  memory.io.clock := clock
  memory.io.reset := reset.asBool
  memory.io.ifuReqValid := cpu.io.ifu.reqValid
  memory.io.ifuAddr := cpu.io.ifu.addr
  cpu.io.ifu.respValid := memory.io.ifuRespValid
  cpu.io.ifu.rdata := memory.io.ifuRdata

  memory.io.lsuReqValid := cpu.io.lsu.reqValid
  memory.io.lsuAddr := cpu.io.lsu.addr
  memory.io.lsuWen := cpu.io.lsu.wen
  memory.io.lsuWdata := cpu.io.lsu.wdata
  memory.io.lsuWmask := cpu.io.lsu.wmask
  cpu.io.lsu.respValid := memory.io.lsuRespValid
  cpu.io.lsu.rdata := memory.io.lsuRdata
}
