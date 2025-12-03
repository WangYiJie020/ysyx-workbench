package top

import chisel3._

import regfile._
import memory._

import cpu._

import chisel3.util.circt.dpi._

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

  val io = IO(new TopIO)
  dontTouch(io)
  io := DontCare

  val gprs = Module(new RegisterFile(READ_PORTS = 2))
  val csrs = Module(new ControlStatusRegisterFile())

  val mem = Module(new MemUnit)

  val ifu = Module(new IFU)
  val idu = Module(new IDU)
  val exu = Module(new EXU)
  val wbu = Module(new WBU)

  val INIT_PC = "h80000000".U(32.W)

  val pc     = RegInit(INIT_PC)

  val is_ebreak = (ifu.io.out.valid) && (ifu.io.out.bits.code === "h00100073".U)

  val nxt_pc = exu.io.out.bits.nxt_pc
  val nxt_pc_valid = exu.io.out.valid

  when(is_ebreak) {
    printf(p"EBREAK at PC = 0x${Hexadecimal(ifu.io.out.bits.pc)} a0 = 0x${Hexadecimal(gprs.io.a0)}\n")
    RawClockedVoidFunctionCall("raise_ebreak")(clock, is_ebreak, gprs.io.a0)
  }

  pc := Mux(wbu.io.done, nxt_pc, pc)

  when(nxt_pc_valid){ 
    printf(p"(Top) PC: 0x${Hexadecimal(pc)} -> 0x${Hexadecimal(nxt_pc)}\n")
    RawClockedVoidFunctionCall("pc_upd")(
      clock,
      nxt_pc_valid,
      pc,nxt_pc
    )
  }

  ifu.io.pc.bits  := pc
  ifu.io.pc.valid := true.B

  ifu.io.out <> idu.io.in
  idu.io.out <> exu.io.dinst

  exu.io.rvec <> gprs.io.read
  exu.io.csr_rvec <> csrs.io.read

  mem.io.read <> exu.io.mem_rreq
  mem.io.write <> exu.io.mem_wreq

  // Write back

  wbu.io.data <> exu.io.out
  gprs.io.write <> wbu.io.gpr
  csrs.io.write <> wbu.io.csr
  csrs.io.is_ecall := wbu.io.is_ecall

}
