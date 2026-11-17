package core

import chisel3._

class RegisterFileIO extends Bundle {
  val rs1Addr = Input(UInt(4.W))
  val rs2Addr = Input(UInt(4.W))
  val rs1Data = Output(UInt(32.W))
  val rs2Data = Output(UInt(32.W))
  val writeEn = Input(Bool())
  val writeAddr = Input(UInt(4.W))
  val writeData = Input(UInt(32.W))
}

/** Two-read-port RV32E register file. The architectural GPRs intentionally have
  * no reset because their reset values are unspecified by the ISA. */
class RegisterFile extends Module {
  val io = IO(new RegisterFileIO)

  private val regs = Reg(Vec(15, UInt(32.W)))

  io.rs1Data := Mux(io.rs1Addr === 0.U, 0.U, regs(io.rs1Addr - 1.U))
  io.rs2Data := Mux(io.rs2Addr === 0.U, 0.U, regs(io.rs2Addr - 1.U))

  when(io.writeEn && io.writeAddr =/= 0.U) {
    regs(io.writeAddr - 1.U) := io.writeData
  }
}
