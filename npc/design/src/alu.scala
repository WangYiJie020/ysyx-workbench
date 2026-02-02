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

class AddSub extends Module {
  val io = IO(new Bundle {
    val src1 = Input(Types.UWord)
    val src2 = Input(Types.UWord)
    val sub = Input(Bool())
    val res  = Output(Types.UWord)
  })
  val op2 = Mux(io.sub, (~io.src2).asUInt + 1.U, io.src2)
  io.res := io.src1 + op2
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

  val add_sub_res = Wire(Types.UWord)
  // when(inbits.is_imm || inbits.func7t === 0.U) {
  //   add_sub_res := src1 + src2
  // }.elsewhen(inbits.func7t === "b0100000".U) {
  //   add_sub_res := src1 - src2
  // }.otherwise {
  //   add_sub_res := BADCALL_RESVALUE
  //   // printf("(alu) UNKNOWN func7t %d", inbits.func7t)
  // }
  
  val addsub = Module(new AddSub)
  addsub.io.src1 := src1
  addsub.io.src2 := src2
  addsub.io.sub  := (!inbits.is_imm) && (inbits.func7t === "b0100000".U)
  add_sub_res := addsub.io.res

  val shift_res = Wire(Types.UWord)
  when(inbits.func7t === "b0100000".U) { // sra/srai
    shift_res := (s_src1 >> shamt).asUInt
  }.otherwise { // srl/srli
    shift_res := src1 >> shamt
  }

  val is_sltiu_func7t = (inbits.func7t === "b0010011".U)

  io.out.bits := MuxLookup(inbits.func3t, BADCALL_RESVALUE)(
    Seq(
      0.U -> add_sub_res,                    // 000: add/sub/addi
      1.U -> (src1 << shamt),                // 001: sll/slli
      2.U -> Mux(s_src1 < s_src2, 1.U, 0.U), // 010: slt/slti
      3.U -> Mux(src1 < src2, 1.U, 0.U),     // 011: sltu/sltiu
      4.U -> (src1 ^ src2),                  // 100: xor/xori
      5.U -> shift_res,                      // 101: srl/srli/sra/srai
      6.U -> (src1 | src2),                  // 110: or/ori
      7.U -> (src1 & src2)                   // 111: and/andi
    )
  )
}
