package core

import chisel3._
import chisel3.util.circt.dpi.RawClockedVoidFunctionCall

class RegisterFileIO extends Bundle {
  val rs1Addr = Input(UInt(4.W))
  val rs2Addr = Input(UInt(4.W))
  val rs1Data = Output(UInt(32.W))
  val rs2Data = Output(UInt(32.W))
  val writeEn = Input(Bool())
  val writeAddr = Input(UInt(4.W))
  val writeData = Input(UInt(32.W))
}

class RegisterFile extends Module {
  val io = IO(new RegisterFileIO)

  private val regs = RegInit(VecInit(Seq.fill(16)(0.U(32.W))))

  io.rs1Data := Mux(io.rs1Addr === 0.U, 0.U, regs(io.rs1Addr))
  io.rs2Data := Mux(io.rs2Addr === 0.U, 0.U, regs(io.rs2Addr))

  when(io.writeEn && io.writeAddr =/= 0.U) {
    regs(io.writeAddr) := io.writeData
    RawClockedVoidFunctionCall("gpr_upd")(clock, true.B, io.writeAddr.pad(8), io.writeData)
  }
}
