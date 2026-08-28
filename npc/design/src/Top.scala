package top

import chisel3._
import chisel3.util._
import chisel3.util.circt.dpi.RawClockedVoidFunctionCall
import config.{CpuConfig, CpuIsa}
import core.{CSRFile, InstructionDecoder, RegisterFile}
import simplebus._

class TopIO extends Bundle {
  val ifu = new SimpleBusIMEM
  val lsu = new SimpleBusDMEM
}

/** A serial, non-pipelined RV32E CPU with one outstanding bus request. */
class ysyx_25100261(config: CpuConfig) extends Module {
  val io = IO(new TopIO)

  private val fullRv32e = config.isa == CpuIsa.RV32E
  private val resetPc = config.resetPc.U(32.W)

  private val pc = RegInit(resetPc)
  private val inst = Reg(UInt(32.W))
  private val nextPc = Reg(UInt(32.W))
  private val rd = Reg(UInt(4.W))
  private val wbData = Reg(UInt(32.W))
  private val wbEn = RegInit(false.B)

  object State extends ChiselEnum {
    val fetchReq, fetchWait, decode, loadWait, storeWait, commit = Value
  }
  private val state = RegInit(State.fetchReq)

  private val regFile = Module(new RegisterFile)
  regFile.io.rs1Addr := inst(19, 15)(3, 0)
  regFile.io.rs2Addr := inst(24, 20)(3, 0)
  regFile.io.writeEn := state === State.commit && wbEn
  regFile.io.writeAddr := rd
  regFile.io.writeData := wbData

  private val csrFile = Module(new CSRFile)
  private val decoder = Module(new InstructionDecoder(config.isa))
  decoder.io.inst := inst
  decoder.io.pc := pc
  decoder.io.rs1Data := regFile.io.rs1Data
  decoder.io.rs2Data := regFile.io.rs2Data
  decoder.io.csrReadData := csrFile.io.readData
  decoder.io.mtvec := csrFile.io.mtvec
  decoder.io.mepc := csrFile.io.mepc
  csrFile.io.readAddr := decoder.io.csrAddr
  csrFile.io.writeEn := state === State.decode && decoder.io.csrWriteEn
  csrFile.io.writeAddr := decoder.io.csrAddr
  csrFile.io.writeData := decoder.io.csrWriteData
  csrFile.io.trapEn := state === State.decode && decoder.io.isEcall
  csrFile.io.trapPc := pc

  io.ifu.reqValid := !reset.asBool && state === State.fetchReq
  io.ifu.addr := pc
  io.lsu.reqValid := !reset.asBool && state === State.decode && (decoder.io.isLoad || decoder.io.isStore)
  io.lsu.addr := decoder.io.memAddr
  io.lsu.size := decoder.io.memSize
  io.lsu.wen := decoder.io.isStore
  io.lsu.wdata := decoder.io.storeData
  io.lsu.wmask := decoder.io.storeMask

  when(reset.asBool) {
    pc := resetPc
    state := State.fetchReq
    wbEn := false.B
  }.otherwise {
    switch(state) {
      is(State.fetchReq) {
        state := State.fetchWait
      }
      is(State.fetchWait) {
        when(io.ifu.respValid) {
          inst := io.ifu.rdata
          state := State.decode
        }
      }
      is(State.decode) {
        rd := decoder.io.rd
        wbEn := false.B
        nextPc := decoder.io.nextPc
        when(decoder.io.isLoad) {
          state := State.loadWait
        }.elsewhen(decoder.io.isStore) {
          state := State.storeWait
        }.otherwise {
          wbData := decoder.io.writeData
          wbEn := decoder.io.writeRd
          state := State.commit
        }
      }
      is(State.loadWait) {
        when(io.lsu.respValid) {
          val isMmio = io.lsu.addr(31, 28) === 1.U
          val shifted = Mux(isMmio, io.lsu.rdata, io.lsu.rdata >> (io.lsu.addr(1, 0) << 3))
          val byte = shifted(7, 0)
          if (fullRv32e) {
            val half = shifted(15, 0)
            wbData := MuxLookup(inst(14, 12), Cat(Fill(24, byte(7)), byte))(Seq(
              "b001".U -> Cat(Fill(16, half(15)), half),
              "b010".U -> shifted,
              "b100".U -> Cat(0.U(24.W), byte),
              "b101".U -> Cat(0.U(16.W), half)
            ))
          } else {
            wbData := Mux(inst(14, 12) === "b010".U, shifted, Cat(0.U(24.W), byte))
          }
          wbEn := true.B
          state := State.commit
        }
      }
      is(State.storeWait) {
        when(io.lsu.respValid) { state := State.commit }
      }
      is(State.commit) {
        pc := nextPc
        wbEn := false.B
        state := State.fetchReq
      }
    }
  }

  when(state === State.commit && decoder.io.isEbreak) {
    RawClockedVoidFunctionCall("raise_ebreak")(clock, true.B)
  }
  when(state === State.commit && !decoder.io.isEbreak) {
    RawClockedVoidFunctionCall("pc_upd")(clock, true.B, pc, nextPc)
  }
  when(state === State.commit && decoder.io.isSystem && !decoder.io.isEbreak) {
    RawClockedVoidFunctionCall("skip_difftest_ref")(clock, true.B)
  }
}
