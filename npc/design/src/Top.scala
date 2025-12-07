package top

import chisel3._

import regfile._
import memory._

import cpu._

import chisel3.util.circt.dpi._
import chisel3.util._

// For NVBoard
class TopIO extends Bundle {
  val btn      = Input(UInt(5.W))
  val sw       = Input(UInt(16.W))
  val ps2_clk  = Input(Bool())
  val ps2_data = Input(Bool())
  val uart_rx  = Input(Bool())
  val uart_tx  = Output(Bool())
  val ledr     = Output(UInt(16.W))

  val VGA_CLK     = Output(Bool())
  val VGA_HSYNC   = Output(Bool())
  val VGA_VSYNC   = Output(Bool())
  val VGA_BLANK_N = Output(Bool())
  val VGA_R       = Output(UInt(8.W))
  val VGA_G       = Output(UInt(8.W))
  val VGA_B       = Output(UInt(8.W))

  val seg0 = Output(UInt(8.W))
  val seg1 = Output(UInt(8.W))
  val seg2 = Output(UInt(8.W))
  val seg3 = Output(UInt(8.W))
  val seg4 = Output(UInt(8.W))
  val seg5 = Output(UInt(8.W))
  val seg6 = Output(UInt(8.W))
  val seg7 = Output(UInt(8.W))
}

// make exu and ifu access memory
class EXUIFU_MemVisitArbiter extends Module {
  val io = IO(new Bundle {
    val exu = AXI4LiteIO.RX
    val ifu = AXI4LiteIO.RX

    val out = AXI4LiteIO.TX
  })

  // Simple arbiter, since IFU and EXU won't access memory at the same time

  val isExuReg = Reg(Bool())
  val isIfuReg = Reg(Bool())

  val isExu = (isExuReg && (!io.ifu.ar.valid)) || (io.exu.ar.valid)
  val isIfu = (isIfuReg && (!io.exu.ar.valid)) || (io.ifu.ar.valid)

  when(io.exu.ar.valid) {
    isExuReg := true.B
    isIfuReg := false.B
  }.elsewhen(io.ifu.ar.valid) {
    isExuReg := false.B
    isIfuReg := true.B
  }

  // AR channel
  io.out.ar.valid := io.exu.ar.valid || io.ifu.ar.valid
  io.out.ar.bits  := Mux(isExu, io.exu.ar.bits, io.ifu.ar.bits)

  io.exu.ar.ready := isExu && io.out.ar.ready
  io.ifu.ar.ready := isIfu && io.out.ar.ready

  // R channel
  io.exu.r.bits <> io.out.r.bits
  io.ifu.r.bits <> io.out.r.bits

  io.exu.r.valid := isExu && io.out.r.valid
  io.ifu.r.valid := isIfu && io.out.r.valid

  io.out.r.ready := Mux(isExu, io.exu.r.ready, io.ifu.r.ready)

  // AW, W, B channel
  //   since only exu need write
  io.out.aw <> io.exu.aw
  io.out.w <> io.exu.w
  io.exu.b <> io.out.b

  io.ifu.aw := DontCare
  io.ifu.w  := DontCare
  io.ifu.b  := DontCare
}

class Top(word_width: Int = 32) extends Module {
  type HasIO = {
    val io: Data
  }
  def instantiateForTest[M <: Module with HasIO](gen: => M): M = {
    val m = Module(gen)
    m.io := DontCare
    dontTouch(m.io)
    m
  }

  // val io = IO(new TopIO)
  // dontTouch(io)
  // io := DontCare

  val gprs = Module(new RegisterFile(READ_PORTS = 2))
  val csrs = Module(new ControlStatusRegisterFile())

  val mem = Module(new AXI4LiteMemUnit)

  val ifu = Module(new IFU)
  val idu = Module(new IDU)
  val exu = Module(new EXU)
  val wbu = Module(new WBU)

  val INIT_PC = "h80000000".U(32.W)

  val pc = RegInit(INIT_PC)

  val is_ebreak = (ifu.io.out.valid) && (ifu.io.out.bits.code === "h00100073".U)

  val nxt_pc       = exu.io.out.bits.nxt_pc
  val nxt_pc_valid = wbu.io.done

  val halted = RegInit(false.B)

  when(is_ebreak && !halted) {
    // printf(p"EBREAK at PC = 0x${Hexadecimal(ifu.io.out.bits.pc)} a0 = 0x${Hexadecimal(gprs.io.a0)}\n")
    // RawClockedVoidFunctionCall("raise_ebreak")(clock, is_ebreak, gprs.io.a0)
    halted := true.B
  }

  pc := Mux(wbu.io.done, nxt_pc, pc)

  when(nxt_pc_valid) {
//    printf(p"(Top) PC: 0x${Hexadecimal(pc)} -> 0x${Hexadecimal(nxt_pc)}\n")
    // RawClockedVoidFunctionCall("pc_upd")(
    //   clock,
    //   nxt_pc_valid,
    //   pc,
    //   nxt_pc
    // )
  }

  val memArbiter = Module(new EXUIFU_MemVisitArbiter)
  mem.io <> memArbiter.io.out
  memArbiter.io.exu <> exu.io.mem
  memArbiter.io.ifu <> ifu.io.mem

  ifu.io.pc.bits  := pc
  ifu.io.pc.valid := true.B

  ifu.io.out <> idu.io.in
  idu.io.out <> exu.io.dinst

  exu.io.rvec <> gprs.io.read
  exu.io.csr_rvec <> csrs.io.read

  // Write back

  wbu.io.data <> exu.io.out
  gprs.io.write <> wbu.io.gpr
  csrs.io.write <> wbu.io.csr
  csrs.io.is_ecall := wbu.io.is_ecall

}
