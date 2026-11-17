package top

import chisel3._
import chisel3.util._
import chisel3.util.HasBlackBoxInline
import chisel3.util.circt.dpi.RawClockedVoidFunctionCall
import config.{CpuConfig, CpuIsa}
import core.{CSRFile, RegisterFile}
import simplebus._

class TopIO extends Bundle {
  val ifu = new SimpleBusIMEM
  val lsu = new SimpleBusDMEM
}

private class NPCTrap extends BlackBox {
  val io = IO(new Bundle {
    val valid = Input(Bool())
    val isMMIO = Input(Bool())
    val isEbreak = Input(Bool())
    val code = Input(UInt(32.W))
  })
}

private class DeIfuPcProbe extends BlackBox with HasBlackBoxInline {
  val io = IO(new Bundle {
    val clock = Input(Clock())
    val pc = Input(UInt(32.W))
  })
  setInline(
    "DeIfuPcProbe.sv",
    """module DeIfuPcProbe(
      |  input clock,
      |  input [31:0] pc
      |); /*verilator no_inline_module*/
      |  (* keep *) reg [31:0] PC /*verilator public_flat_rw*/;
      |  always @(posedge clock) PC <= pc;
      |endmodule
      |""".stripMargin
  )
}

private class IcarusMonitor extends BlackBox with HasBlackBoxInline {
  val io = IO(new Bundle {
    val clock = Input(Clock())
    val reset = Input(Bool())
    val commit = Input(Bool())
    val ebreak = Input(Bool())
    val writeEn = Input(Bool())
    val writeAddr = Input(UInt(4.W))
    val writeData = Input(UInt(32.W))
  })

  setInline(
    "IcarusMonitor.sv",
    """module IcarusMonitor(
      |  input clock, input reset, input commit, input ebreak,
      |  input writeEn, input [3:0] writeAddr, input [31:0] writeData
      |);
      |  reg [31:0] haltCode;
      |  always @(posedge clock) begin
      |    if (reset) begin
      |      haltCode <= 32'd0;
      |    end else begin
      |      if (commit && writeEn && writeAddr == 4'd10) haltCode <= writeData;
      |      if (commit && ebreak) begin
      |        $display("NPC ebreak: halt code = %0d", haltCode);
      |        if (haltCode == 32'd0) $finish;
      |        else $fatal(1, "NPC program failed with halt code %0d", haltCode);
      |      end
      |    end
      |  end
      |endmodule
      |""".stripMargin
  )
}

/** Non-pipelined RV32E core with two-port GPR reads and single-cycle arithmetic. */
class ysyx_25100261(config: CpuConfig) extends Module {
  val io = IO(new TopIO)
  private val fullRv32e = config.isa == CpuIsa.RV32E

  private val pc = RegInit(config.resetPc.U(32.W))
  private val inst = Reg(UInt(32.W))
  private val operand1 = Reg(UInt(32.W))
  private val operand2 = Reg(UInt(32.W))
  private val result = Reg(UInt(32.W))
  private val step = Reg(UInt(5.W))

  object State extends ChiselEnum {
    val fetchReq, fetchWait, readOperands, execute, shiftStep,
      executeDone, memoryReq, memoryWait, commit = Value
  }
  private val state = RegInit(State.fetchReq)

  private val opcode = inst(6, 0)
  private val funct3 = inst(14, 12)
  private val funct7Bit = inst(30)
  private val rd = inst(11, 7)(3, 0)
  private val rs1Addr = inst(19, 15)(3, 0)
  private val immI = Cat(Fill(20, inst(31)), inst(31, 20))
  private val immS = Cat(Fill(20, inst(31)), inst(31, 25), inst(11, 7))
  private val immB = Cat(Fill(19, inst(31)), inst(31), inst(7), inst(30, 25), inst(11, 8), 0.U)
  private val immJ = Cat(Fill(11, inst(31)), inst(31), inst(19, 12), inst(20), inst(30, 21), 0.U)
  private val immU = Cat(inst(31, 12), 0.U(12.W))

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
  private val isLoad = opcode === "b0000011".U && loadFunctValid
  private val isStore = opcode === "b0100011".U && storeFunctValid
  private val isJal = fullRv32e.B && opcode === "b1101111".U
  private val isJalr = opcode === "b1100111".U && funct3 === 0.U
  private val isBranch = fullRv32e.B && opcode === "b1100011".U
  private val isEcall = inst === "h00000073".U
  private val isEbreak = inst === "h00100073".U
  private val isMret = inst === "h30200073".U
  private val isCsr = opcode === "b1110011".U && funct3 =/= 0.U

  private val regFile = Module(new RegisterFile)
  regFile.io.rs1Addr := rs1Addr
  regFile.io.rs2Addr := inst(24, 20)(3, 0)

  private val csrFile = Module(new CSRFile)
  csrFile.io.readAddr := inst(31, 20)

  private val branchTaken = MuxLookup(funct3, operand1 === operand2)(Seq(
    "b001".U -> (operand1 =/= operand2),
    "b100".U -> (operand1.asSInt < operand2.asSInt),
    "b101".U -> (operand1.asSInt >= operand2.asSInt),
    "b110".U -> (operand1 < operand2),
    "b111".U -> (operand1 >= operand2)
  ))
  private val opImmediate = opcode === "b0010011".U
  private val opRegister = opcode === "b0110011".U
  private val isShift = if (fullRv32e) {
    (opImmediate || opRegister) && (funct3 === "b001".U || funct3 === "b101".U)
  } else false.B
  private val isSetLess = if (fullRv32e) {
    (opImmediate || opRegister) && (funct3 === "b010".U || funct3 === "b011".U)
  } else false.B
  private val addNeeded = if (fullRv32e) {
    isLoad || isStore || isJal || isJalr || opcode === "b0010111".U ||
      (isBranch && branchTaken) || (opImmediate && funct3 === 0.U) ||
      (opRegister && funct3 === 0.U) || isSetLess
  } else {
    isLoad || isStore || isJalr || (opImmediate && funct3 === 0.U) ||
      (opRegister && funct3 === 0.U && !funct7Bit)
  }
  private val addLhs = if (fullRv32e) {
    Mux(isJal || (isBranch && branchTaken) || opcode === "b0010111".U, pc, operand1)
  } else operand1
  private val addRhs = if (fullRv32e) {
    MuxLookup(opcode, operand2)(Seq(
      "b0000011".U -> immI,
      "b0100011".U -> immS,
      "b0010011".U -> immI,
      "b0010111".U -> immU,
      "b1100011".U -> immB,
      "b1100111".U -> immI,
      "b1101111".U -> immJ
    ))
  } else {
    Mux(opRegister, operand2, Mux(isStore, immS, immI))
  }
  private val addSubtract = if (fullRv32e) {
    (opRegister && funct3 === 0.U && funct7Bit) || isSetLess
  } else false.B
  private val addResult = addLhs + (addRhs ^ Fill(32, addSubtract)) + addSubtract
  private val directResult = if (fullRv32e) {
    MuxLookup(funct3, 0.U(32.W))(Seq(
      "b100".U -> (operand1 ^ Mux(opImmediate, immI, operand2)),
      "b110".U -> (operand1 | Mux(opImmediate, immI, operand2)),
      "b111".U -> (operand1 & Mux(opImmediate, immI, operand2))
    ))
  } else 0.U(32.W)

  private val writeRd = if (fullRv32e) {
    isLoad || isCsr || isJal || isJalr || opcode === "b0110111".U ||
      opcode === "b0010111".U || opcode === "b0010011".U || opcode === "b0110011".U
  } else {
    isLoad || isCsr || isJalr || opcode === "b0110111".U ||
      (opImmediate && funct3 === 0.U) ||
      (opRegister && funct3 === 0.U && !funct7Bit)
  }
  private val writeData = Mux(isJal || isJalr, pc + 4.U, result)
  private val committing = state === State.commit
  regFile.io.writeEn := committing && writeRd
  regFile.io.writeAddr := rd
  regFile.io.writeData := writeData

  private val csrSource = Mux(funct3(2), inst(19, 15), operand1)
  private val csrWriteEn = isCsr && (funct3(1, 0) === 1.U || rs1Addr =/= 0.U)
  private val csrWriteData = MuxLookup(funct3(1, 0), csrSource)(Seq(
    "b10".U -> (csrFile.io.readData | csrSource),
    "b11".U -> (csrFile.io.readData & ~csrSource)
  ))
  csrFile.io.writeEn := committing && csrWriteEn
  csrFile.io.writeAddr := inst(31, 20)
  csrFile.io.writeData := operand2
  csrFile.io.trapEn := committing && isEcall
  csrFile.io.trapPc := pc

  private val memSize = if (fullRv32e) {
    MuxLookup(funct3(1, 0), "b10".U)(Seq("b00".U -> "b00".U, "b01".U -> "b01".U))
  } else Mux(funct3 === 0.U, "b00".U, "b10".U)
  private val storeData = if (fullRv32e) {
    MuxLookup(funct3(1, 0), operand2)(Seq(
      "b00".U -> (operand2(7, 0) << (result(1, 0) << 3)),
      "b01".U -> (operand2(15, 0) << (result(1) << 4))
    ))
  } else {
    val byteData = MuxLookup(result(1, 0), Cat(0.U(24.W), operand2(7, 0)))(Seq(
      1.U -> Cat(0.U(16.W), operand2(7, 0), 0.U(8.W)),
      2.U -> Cat(0.U(8.W), operand2(7, 0), 0.U(16.W)),
      3.U -> Cat(operand2(7, 0), 0.U(24.W))
    ))
    Mux(funct3 === 0.U, byteData, operand2)
  }
  private val storeMask = if (fullRv32e) {
    MuxLookup(funct3(1, 0), "b1111".U)(Seq(
      "b00".U -> (1.U(4.W) << result(1, 0)),
      "b01".U -> (3.U(4.W) << Cat(result(1), 0.U(1.W)))
    ))
  } else {
    val byteMask = MuxLookup(result(1, 0), "b0001".U(4.W))(Seq(
      1.U -> "b0010".U,
      2.U -> "b0100".U,
      3.U -> "b1000".U
    ))
    Mux(funct3 === 0.U, byteMask, "b1111".U)
  }

  io.ifu.reqValid := !reset.asBool && state === State.fetchReq
  io.ifu.addr := pc
  io.lsu.reqValid := !reset.asBool && state === State.memoryReq
  io.lsu.addr := result
  io.lsu.size := memSize
  io.lsu.wen := isStore
  io.lsu.wdata := storeData
  io.lsu.wmask := storeMask

  private val nextPc = if (fullRv32e) {
    Mux(isEcall, csrFile.io.mtvec,
      Mux(isMret, csrFile.io.mepc, Mux(isJal || isJalr || isBranch, result, pc + 4.U)))
  } else {
    Mux(isEcall, csrFile.io.mtvec, Mux(isMret, csrFile.io.mepc, Mux(isJalr, result, pc + 4.U)))
  }

  when(reset.asBool) {
    pc := config.resetPc.U
    state := State.fetchReq
  }.otherwise {
    switch(state) {
      is(State.fetchReq) { state := State.fetchWait }
      is(State.fetchWait) {
        when(io.ifu.respValid) {
          inst := io.ifu.rdata
          state := State.readOperands
        }
      }
      is(State.readOperands) {
        operand1 := regFile.io.rs1Data
        operand2 := regFile.io.rs2Data
        state := State.execute
      }
      is(State.execute) {
        when(addNeeded) {
          result := Mux(isSetLess,
            Mux(funct3 === "b010".U, operand1.asSInt < addRhs.asSInt, operand1 < addRhs).asUInt,
            addResult)
          state := State.executeDone
        }.elsewhen(isShift) {
          result := operand1
          step := Mux(opImmediate, inst(24, 20), operand2(4, 0))
          state := State.shiftStep
        }.otherwise {
          result := Mux(isCsr, csrFile.io.readData,
            Mux(opcode === "b0110111".U, immU, Mux(isBranch, pc + 4.U, directResult)))
          when(isCsr) {
            operand2 := csrWriteData
          }
          state := State.executeDone
        }
      }
      is(State.shiftStep) {
        if (fullRv32e) {
          when(step === 0.U) {
            state := State.executeDone
          }.otherwise {
            result := Mux(funct3 === "b001".U, Cat(result(30, 0), 0.U),
              Mux(funct7Bit, Cat(result(31), result(31, 1)), Cat(0.U, result(31, 1))))
            step := step - 1.U
          }
        }
      }
      is(State.executeDone) {
        when(isJalr) {
          result := result & "hfffffffe".U
        }
        state := Mux(isLoad || isStore, State.memoryReq, State.commit)
      }
      is(State.memoryReq) { state := State.memoryWait }
      is(State.memoryWait) {
        when(io.lsu.respValid) {
          when(isLoad) {
            val isMmio = result(31, 28) === 1.U
            val shifted = Mux(isMmio, io.lsu.rdata, io.lsu.rdata >> (result(1, 0) << 3))
            val byte = shifted(7, 0)
            if (fullRv32e) {
              val half = shifted(15, 0)
              result := MuxLookup(funct3, Cat(Fill(24, byte(7)), byte))(Seq(
                "b001".U -> Cat(Fill(16, half(15)), half),
                "b010".U -> shifted,
                "b100".U -> Cat(0.U(24.W), byte),
                "b101".U -> Cat(0.U(16.W), half)
              ))
            } else {
              val selectedByte = MuxLookup(result(1, 0), io.lsu.rdata(7, 0))(Seq(
                1.U -> io.lsu.rdata(15, 8),
                2.U -> io.lsu.rdata(23, 16),
                3.U -> io.lsu.rdata(31, 24)
              ))
              val loadByte = Mux(isMmio, io.lsu.rdata(7, 0), selectedByte)
              result := Mux(funct3 === "b010".U, io.lsu.rdata, Cat(0.U(24.W), loadByte))
            }
          }
          state := State.commit
        }
      }
      is(State.commit) {
        pc := nextPc
        state := State.fetchReq
      }
    }
  }

  if (config.enableDpi) {
    when(committing && isEbreak) {
      RawClockedVoidFunctionCall("raise_ebreak")(clock, true.B)
    }
    when(committing && writeRd && rd =/= 0.U) {
      RawClockedVoidFunctionCall("gpr_upd")(clock, true.B, rd.pad(8), writeData)
    }
    when(committing && !isEbreak) {
      RawClockedVoidFunctionCall("pc_upd")(clock, true.B, pc, nextPc)
    }
  }

  if (config.enableIcarus) {
    val monitor = Module(new IcarusMonitor)
    monitor.io.clock := clock
    monitor.io.reset := reset.asBool
    monitor.io.commit := committing
    monitor.io.ebreak := isEbreak
    monitor.io.writeEn := writeRd
    monitor.io.writeAddr := rd
    monitor.io.writeData := writeData
  }

  if (config.enableNpcTrap) {
    val haltCode = RegInit(0.U(32.W))
    when(committing && writeRd && rd === 10.U) {
      haltCode := writeData
    }
    val trap = Module(new NPCTrap)
    trap.io.valid := committing
    trap.io.isMMIO := false.B
    trap.io.isEbreak := isEbreak
    trap.io.code := Mux(committing && writeRd && rd === 10.U, writeData, haltCode)
    val ifu = Module(new DeIfuPcProbe)
    ifu.suggestName("ifu")
    ifu.io.clock := clock
    ifu.io.pc := pc
  }
}
