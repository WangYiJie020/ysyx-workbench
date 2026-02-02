package cpu.alu

import chisel3._
import chisel3.util._
import common_def._
import busfsm._

class ALUInput extends Bundle {
  val is_imm = Bool()
  val func3t = UInt(3.W)
  val func7t = UInt(7.W)
  val src1   = Types.UWord
  val src2   = Types.UWord
}

class ALU extends Module {
  val io               = IO(new Bundle {
    val in  = Flipped(Decoupled(new ALUInput))
    val out = Decoupled(Types.UWord)
  })
  val BADCALL_RESVALUE = "hBAADCA11".U

  val fsm = Module(new OneMasterOneSlaveFSM)
  fsm.connectMaster(io.in)
  fsm.connectSlave(io.out)
  fsm.io.self_finished := true.B

  // alias
  val inbits = io.in.bits
  val src1   = inbits.src1
  val src2   = inbits.src2

  val s_src1 = src1.asSInt
  val s_src2 = src2.asSInt

  val shamt = src2(4, 0)

  val isAdd = (inbits.func7t === 0.U) || inbits.is_imm

  val add_sub_res = Wire(Types.UWord)
  add_sub_res := Mux(isAdd, src1 + src2, src1 - src2)

  val isLessThanU = Wire(Bool())
  isLessThanU := Mux(src1 < src2, 1.U, 0.U)

  val shift_res = Wire(Types.UWord)
  when(inbits.func7t === "b0100000".U) { // sra/srai
    shift_res := (s_src1 >> shamt).asUInt
  }.otherwise { // srl/srli
    shift_res := src1 >> shamt
  }

  io.out.bits := MuxLookup(inbits.func3t, BADCALL_RESVALUE)(
    Seq(
      0.U -> add_sub_res,                    // 000: add/sub/addi
      1.U -> (src1 << shamt),                // 001: sll/slli
      2.U -> Mux(s_src1 < s_src2, 1.U, 0.U), // 010: slt/slti
      3.U -> isLessThanU,                    // 011: sltu/sltiu
      4.U -> (src1 ^ src2),                  // 100: xor/xori
      5.U -> shift_res,                      // 101: srl/srli/sra/srai
      6.U -> (src1 | src2),                  // 110: or/ori
      7.U -> (src1 & src2)                   // 111: and/andi
    )
  )
}
