package cpu

import chisel3._
import chisel3.util._

import chisel3.experimental.dataview._

import common_def._
import busfsm._
import regfile._

import cpu.WriteBackInfo

class InstInfoDecoder extends Module {
  val io = IO(new Bundle {
    val opcode = Input(UInt(7.W))
    val valid  = Output(Bool())
    val out    = Output(new InstMetaInfo())
  })

  // opcode[1:0] should always be 11 for 32bit
  io.valid := io.opcode(1, 0).andR
  val opcu = io.opcode(6, 2)

  val lut = Seq(
    "b00000".U -> (InstFmt.imm, InstType.load),
    "b00100".U -> (InstFmt.imm, InstType.arithmetic),
    "b11001".U -> (InstFmt.imm, InstType.jalr),
    "b11100".U -> (InstFmt.imm, InstType.system),
    "b01100".U -> (InstFmt.reg, InstType.arithmetic),
    "b01000".U -> (InstFmt.store, InstType.store),
    "b01101".U -> (InstFmt.upper, InstType.lui),
    "b00101".U -> (InstFmt.upper, InstType.auipc),
    "b11011".U -> (InstFmt.jump, InstType.jal),
    "b11000".U -> (InstFmt.branch, InstType.branch),
    "b00011".U -> (InstFmt.imm, InstType.fencei)
  ).map { case (key, (fmt, typ)) =>
    key -> {
      val info = Wire(new InstMetaInfo)
      info.fmt := fmt
      info.typ := typ
      info
    }
  }

  val dontcare = Wire(new InstMetaInfo)
  dontcare := DontCare
  io.out   := MuxLookup(opcu, dontcare)(lut)
}

class IDU extends Module {
  val io = IO(new Bundle {
    val in   = Flipped(Decoupled(new Inst))
    val rvec = GPRegReqIO.TX.VecRead(2)

    val flush = Input(Bool())

    // val r1UseBypass = Input(Bool())
    // val r2UseBypass = Input(Bool())
    //
    // val r1BypassData = Input(Types.UWord)
    // val r2BypassData = Input(Types.UWord)

    // connect to EXU output
    val exuWrBack          = GPRegReqIO.RX.Write
    // connect to EXU output, indicate whether the data is ready for bypassing
    //
    // for memory operations, the data is not ready until the memory access is
    // finished
    val exuWrBackDataVaild = Input(Bool())

    // !!! Notice
    // connect to LSU **input**
    val lsuWrBack = GPRegReqIO.RX.Write

    val wbuWrBack = GPRegReqIO.RX.Write

    val out = Decoupled(new DecodedInst)
  })

  dontTouch(io)

  // val inReg = RegEnable(io.in.bits, io.in.fire)
  val inReg = io.in.bits

  // TODO: handle invalid instruction

  // val fsm = InnerBusCtrl(io.in, io.out, alwaysComb = true)

  io.out.bits.viewAsSupertype(new Inst) := inReg

  // alias
  val res  = io.out.bits.info
  val inst = inReg.code

  val iinfo_dec = Module(new InstInfoDecoder())
  iinfo_dec.io.opcode                   := inst(6, 0)
  res.viewAsSupertype(new InstMetaInfo) := iinfo_dec.io.out

  res.rd  := inst(11, 7)
  res.rs1 := inst(19, 15)
  res.rs2 := inst(24, 20)

  io.rvec.en      := true.B
  io.rvec.addr(0) := res.rs1
  io.rvec.addr(1) := res.rs2

  // conflict logic

  def conflict(rs: UInt, rd: UInt, en: Bool): Bool = (rs === rd) && (rd =/= 0.U) && en

  def conflict(rs1: UInt, rs2: UInt, gprWr: GPRegReqIO._WriteRX): Bool = {
    (conflict(rs1, gprWr.addr, gprWr.en) || conflict(rs2, gprWr.addr, gprWr.en))
  }
  def conflict[T <: HasRs](info: T, gprWr: GPRegReqIO._WriteRX):  Bool = {
    conflict(info.rs1, info.rs2, gprWr)
  }

  val isConflictWithEXU    = Wire(Bool())
  val isRs1ConflictWithEXU = conflict(
    io.out.bits.info.rs1,
    io.exuWrBack.addr,
    io.exuWrBack.en
  )
  val isRs2ConflictWithEXU = conflict(
    io.out.bits.info.rs2,
    io.exuWrBack.addr,
    io.exuWrBack.en
  )
  isConflictWithEXU := (isRs1ConflictWithEXU || isRs2ConflictWithEXU)

  val isConflictWithLSU = Wire(Bool())
  isConflictWithLSU := conflict(io.out.bits.info, io.lsuWrBack)

  val isConflictWithWBU    = Wire(Bool())
  val isRs1ConflictWithWBU = conflict(
    io.out.bits.info.rs1,
    io.wbuWrBack.addr,
    io.wbuWrBack.en
  )
  val isRs2ConflictWithWBU = conflict(
    io.out.bits.info.rs2,
    io.wbuWrBack.addr,
    io.wbuWrBack.en
  )
  isConflictWithWBU := (isRs1ConflictWithWBU || isRs2ConflictWithWBU)

  dontTouch(isConflictWithEXU)
  dontTouch(isConflictWithLSU)
  dontTouch(isConflictWithWBU)

  val isRdAfterWr = Wire(Bool())
  isRdAfterWr := isConflictWithEXU || isConflictWithLSU || isConflictWithWBU
  dontTouch(isRdAfterWr)

  // val exuWrBackDataVaild = !(exu.io.out.bits.isLoad || exu.io.out.bits.isStore)
  val exuWrBackDataVaild = io.exuWrBackDataVaild

  val canRs1Bypass =
    Mux(isRs1ConflictWithEXU, exuWrBackDataVaild, isRs1ConflictWithWBU)
  val canRs2Bypass =
    Mux(isRs2ConflictWithEXU, exuWrBackDataVaild, isRs2ConflictWithWBU)

  val needStallForRs1 = (isRs1ConflictWithEXU || isRs1ConflictWithWBU) && (!canRs1Bypass)
  val needStallForRs2 = (isRs2ConflictWithEXU || isRs2ConflictWithWBU) && (!canRs2Bypass)

  val needStall = needStallForRs1 || needStallForRs2 || isConflictWithLSU

  val IsStall = WireDefault(needStall)
  dontTouch(IsStall)

  val r1UseBypass = canRs1Bypass
  val r2UseBypass = canRs2Bypass

  val r1BypassData = Mux(isRs1ConflictWithEXU, io.exuWrBack.data, io.wbuWrBack.data)
  val r2BypassData = Mux(isRs2ConflictWithEXU, io.exuWrBack.data, io.wbuWrBack.data)

  res.reg1 := Mux(r1UseBypass, r1BypassData, io.rvec.data(0))
  res.reg2 := Mux(r2UseBypass, r2BypassData, io.rvec.data(1))

  // fetch IMM
  val immI = Cat(Fill(21, inst(31)), inst(30, 20))
  val immS = Cat(immI(31, 5), inst(11, 8), inst(7))
  val immB = Cat(immI(31, 12), inst(7), immS(10, 1), 0.U(1.W))
  val immU = Cat(inst(31, 12), 0.U(12.W))
  val immJ = Cat(immI(31, 20), inst(19, 12), inst(20), inst(30, 21), 0.U(1.W))

  val dontcareImm = Wire(Types.UWord)
  dontcareImm := DontCare

  res.imm := MuxLookup(iinfo_dec.io.out.fmt, dontcareImm)(
    Seq(
      InstFmt.imm    -> immI,
      InstFmt.jump   -> immJ,
      InstFmt.store  -> immS,
      InstFmt.branch -> immB,
      InstFmt.upper  -> immU
    )
  )

  io.in.ready  := (io.out.ready && !needStall) || io.flush
  io.out.valid := io.in.valid && !needStall && !io.flush
}
