package core

import chisel3._
import chisel3.util._
import config.CpuIsa

class InstructionDecoderIO extends Bundle {
  val inst = Input(UInt(32.W))
  val pc = Input(UInt(32.W))
  val rs1Data = Input(UInt(32.W))
  val rs2Data = Input(UInt(32.W))
  val csrReadData = Input(UInt(32.W))
  val mtvec = Input(UInt(32.W))
  val mepc = Input(UInt(32.W))
  val rd = Output(UInt(4.W))
  val writeRd = Output(Bool())
  val writeData = Output(UInt(32.W))
  val nextPc = Output(UInt(32.W))
  val isLoad = Output(Bool())
  val isStore = Output(Bool())
  val memAddr = Output(UInt(32.W))
  val memSize = Output(UInt(2.W))
  val storeData = Output(UInt(32.W))
  val storeMask = Output(UInt(4.W))
  val isEcall = Output(Bool())
  val isEbreak = Output(Bool())
  val isSystem = Output(Bool())
  val csrAddr = Output(UInt(12.W))
  val csrWriteEn = Output(Bool())
  val csrWriteData = Output(UInt(32.W))
}

class InstructionDecoder(isa: CpuIsa) extends Module {
  val io = IO(new InstructionDecoderIO)

  private val fullRv32e = isa == CpuIsa.RV32E
  private val opcode = io.inst(6, 0)
  private val funct3 = io.inst(14, 12)
  private val funct7Bit = io.inst(30)
  private val rs1Addr = io.inst(19, 15)
  private val immI = Cat(Fill(20, io.inst(31)), io.inst(31, 20))
  private val immS = Cat(Fill(20, io.inst(31)), io.inst(31, 25), io.inst(11, 7))
  private val immU = Cat(io.inst(31, 12), 0.U(12.W))
  private val immB = Cat(Fill(19, io.inst(31)), io.inst(31), io.inst(7), io.inst(30, 25), io.inst(11, 8), 0.U)
  private val immJ = Cat(Fill(11, io.inst(31)), io.inst(31), io.inst(19, 12), io.inst(20), io.inst(30, 21), 0.U)

  private val loadFunctValid = if (fullRv32e) {
    funct3 === "b000".U || funct3 === "b001".U || funct3 === "b010".U ||
      funct3 === "b100".U || funct3 === "b101".U
  } else {
    funct3 === "b010".U || funct3 === "b100".U
  }
  private val storeFunctValid = if (fullRv32e) {
    funct3 === "b000".U || funct3 === "b001".U || funct3 === "b010".U
  } else {
    funct3 === "b000".U || funct3 === "b010".U
  }

  io.rd := io.inst(11, 7)(3, 0)
  io.writeRd := false.B
  io.writeData := 0.U
  io.nextPc := io.pc + 4.U
  io.isLoad := opcode === "b0000011".U && loadFunctValid
  io.isStore := opcode === "b0100011".U && storeFunctValid
  io.memAddr := io.rs1Data + Mux(io.isStore, immS, immI)
  io.memSize := MuxLookup(funct3(1, 0), "b10".U)(Seq(
    "b00".U -> "b00".U,
    "b01".U -> "b01".U
  ))

  private val byteShift = io.memAddr(1, 0) << 3
  private val halfShift = io.memAddr(1) << 4
  io.storeData := MuxLookup(funct3(1, 0), io.rs2Data)(Seq(
    "b00".U -> (io.rs2Data(7, 0) << byteShift),
    "b01".U -> (io.rs2Data(15, 0) << halfShift)
  ))
  io.storeMask := MuxLookup(funct3(1, 0), "b1111".U)(Seq(
    "b00".U -> (1.U(4.W) << io.memAddr(1, 0)),
    "b01".U -> (3.U(4.W) << Cat(io.memAddr(1), 0.U(1.W)))
  ))

  io.isEcall := io.inst === "h00000073".U
  io.isEbreak := io.inst === "h00100073".U
  private val isMret = io.inst === "h30200073".U
  private val isCsr = opcode === "b1110011".U && funct3 =/= 0.U
  io.isSystem := io.isEcall || io.isEbreak || isMret || isCsr
  io.csrAddr := io.inst(31, 20)
  io.csrWriteEn := false.B
  io.csrWriteData := 0.U

  when(io.isEcall) {
    io.nextPc := io.mtvec
  }.elsewhen(isMret) {
    io.nextPc := io.mepc
  }.elsewhen(isCsr) {
    val source = Mux(funct3(2), rs1Addr, io.rs1Data)
    io.writeRd := true.B
    io.writeData := io.csrReadData
    switch(funct3) {
      is("b001".U) {
        io.csrWriteEn := true.B
        io.csrWriteData := source
      }
      is("b010".U) {
        io.csrWriteEn := rs1Addr =/= 0.U
        io.csrWriteData := io.csrReadData | source
      }
      is("b011".U) {
        io.csrWriteEn := rs1Addr =/= 0.U
        io.csrWriteData := io.csrReadData & ~source
      }
      is("b101".U) {
        io.csrWriteEn := true.B
        io.csrWriteData := source
      }
      is("b110".U) {
        io.csrWriteEn := rs1Addr =/= 0.U
        io.csrWriteData := io.csrReadData | source
      }
      is("b111".U) {
        io.csrWriteEn := rs1Addr =/= 0.U
        io.csrWriteData := io.csrReadData & ~source
      }
    }
  }.otherwise {
    if (fullRv32e) decodeRv32e() else decodeMiniRv()
  }

  private def decodeMiniRv(): Unit = {
    when(opcode === "b0010011".U && funct3 === "b000".U) {
      io.writeData := io.rs1Data + immI
      io.writeRd := true.B
    }.elsewhen(opcode === "b0110011".U && funct3 === "b000".U && !funct7Bit) {
      io.writeData := io.rs1Data + io.rs2Data
      io.writeRd := true.B
    }.elsewhen(opcode === "b0110111".U) {
      io.writeData := immU
      io.writeRd := true.B
    }.elsewhen(opcode === "b1100111".U && funct3 === "b000".U) {
      io.writeData := io.pc + 4.U
      io.writeRd := true.B
      io.nextPc := (io.rs1Data + immI) & "hfffffffe".U
    }
  }

  private def decodeRv32e(): Unit = {
    when(opcode === "b0010011".U) {
      val shamt = io.inst(24, 20)
      io.writeData := MuxLookup(funct3, io.rs1Data + immI)(Seq(
        "b010".U -> (io.rs1Data.asSInt < immI.asSInt).asUInt,
        "b011".U -> (io.rs1Data < immI),
        "b100".U -> (io.rs1Data ^ immI),
        "b110".U -> (io.rs1Data | immI),
        "b111".U -> (io.rs1Data & immI),
        "b001".U -> (io.rs1Data << shamt),
        "b101".U -> Mux(funct7Bit, (io.rs1Data.asSInt >> shamt).asUInt, io.rs1Data >> shamt)
      ))
      io.writeRd := true.B
    }.elsewhen(opcode === "b0110011".U) {
      io.writeData := MuxLookup(funct3, Mux(funct7Bit, io.rs1Data - io.rs2Data, io.rs1Data + io.rs2Data))(Seq(
        "b001".U -> (io.rs1Data << io.rs2Data(4, 0)),
        "b010".U -> (io.rs1Data.asSInt < io.rs2Data.asSInt).asUInt,
        "b011".U -> (io.rs1Data < io.rs2Data),
        "b100".U -> (io.rs1Data ^ io.rs2Data),
        "b101".U -> Mux(funct7Bit, (io.rs1Data.asSInt >> io.rs2Data(4, 0)).asUInt, io.rs1Data >> io.rs2Data(4, 0)),
        "b110".U -> (io.rs1Data | io.rs2Data),
        "b111".U -> (io.rs1Data & io.rs2Data)
      ))
      io.writeRd := true.B
    }.elsewhen(opcode === "b0110111".U) {
      io.writeData := immU
      io.writeRd := true.B
    }.elsewhen(opcode === "b0010111".U) {
      io.writeData := io.pc + immU
      io.writeRd := true.B
    }.elsewhen(opcode === "b1101111".U) {
      io.writeData := io.pc + 4.U
      io.writeRd := true.B
      io.nextPc := io.pc + immJ
    }.elsewhen(opcode === "b1100111".U && funct3 === "b000".U) {
      io.writeData := io.pc + 4.U
      io.writeRd := true.B
      io.nextPc := (io.rs1Data + immI) & "hfffffffe".U
    }.elsewhen(opcode === "b1100011".U) {
      val taken = MuxLookup(funct3, io.rs1Data === io.rs2Data)(Seq(
        "b001".U -> (io.rs1Data =/= io.rs2Data),
        "b100".U -> (io.rs1Data.asSInt < io.rs2Data.asSInt),
        "b101".U -> (io.rs1Data.asSInt >= io.rs2Data.asSInt),
        "b110".U -> (io.rs1Data < io.rs2Data),
        "b111".U -> (io.rs1Data >= io.rs2Data)
      ))
      io.nextPc := Mux(taken, io.pc + immB, io.pc + 4.U)
    }
  }
}
