package cpu

import chisel3._
import chisel3.util.{Cat, Decoupled, DecoupledIO, Enum, Fill, MuxLookup}
import chisel3.util.HasBlackBoxInline

// see https://www.chisel-lang.org/api/latest/chisel3/util/circt/dpi/index.html
// chisel has native dpi interface since
//  https://github.com/chipsalliance/chisel/pull/4158
import chisel3.util.circt.dpi.{
  RawClockedNonVoidFunctionCall,
  RawClockedVoidFunctionCall,
  RawUnclockedNonVoidFunctionCall
}

import chisel3.experimental.dataview._

import regfile._

object Types {
  object BitWidth {
    val reg_addr = 5
    val word     = 32
  }
  def UWord = UInt(BitWidth.word.W)
  def RegAddr = UInt(BitWidth.reg_addr.W)

  object Ops {
    implicit class StringOps(val s: String) extends AnyVal {
      def UWord = s.U(BitWidth.word.W)
    }
    implicit class IntOps(val s: Int)       extends AnyVal {
      def UWord = s.U(BitWidth.word.W)
    }
  }
}
import Types.Ops._

class Inst extends Bundle {
  val code = Output(Types.UWord)
  val pc   = Output(Types.UWord)
}

class BusMasterFSM extends Module {
  val io = IO(new Bundle {
    val want_send   = Input(Bool())
    val slave_ready = Input(Bool())
    val valid       = Output(Bool())
  })

  val s_idle :: s_wait_ready :: Nil = Enum(2)

  val state = RegInit(s_idle)
  state    := MuxLookup(state, s_idle)(
    List(
      s_idle       -> Mux(io.want_send, s_wait_ready, s_idle),
      s_wait_ready -> Mux(io.slave_ready, s_idle, s_wait_ready)
    )
  )
  io.valid := (state === s_idle)

  def connectSlave[T <: Data](slave: DecoupledIO[T]): Unit = {
    io.slave_ready := slave.ready
    slave.valid    := io.valid
  }
}
class BusSlaveFSM  extends Module {
  val io = IO(new Bundle {
    val want_recv    = Input(Bool())
    val master_valid = Input(Bool())
    // ready to recv
    val ready        = Output(Bool())
  })

  val s_idle :: s_wait_data :: Nil = Enum(2)

  val state = RegInit(s_idle)
  state    := MuxLookup(state, s_idle)(
    List(
      s_idle      -> Mux(io.want_recv, s_wait_data, s_idle),
      s_wait_data -> Mux(io.master_valid, s_idle, s_wait_data)
    )
  )
  io.ready := (state === s_idle)

  def connectMaster[T <: Data](master: DecoupledIO[T]): Unit = {
    io.master_valid := master.valid
    master.ready    := io.ready
  }
}

class IFU extends Module {
  val io = IO(new Bundle {
    val pc  = Input(Types.UWord)
    val out = Decoupled(new Inst)
  })

  val send_fsm = Module(new BusMasterFSM)
  send_fsm.io.want_send := 1.B
  send_fsm.connectSlave(io.out)

  // NOTICE: dpi function auto generated with void return
  // see https://github.com/llvm/circt/blob/main/docs/Dialects/FIRRTL/FIRRTLIntrinsics.md#dpi-intrinsic-abi
  io.out.bits.code := RawClockedNonVoidFunctionCall("fetch_inst", Types.UWord)(clock, io.out.valid, io.pc)
  io.out.bits.pc   := io.pc
}

object InstFmt     extends ChiselEnum {
  val imm, reg, store, upper, jump, branch = Value
}
object InstType    extends ChiselEnum {
  val none, arithmetic, load, jalr, jal, lui, auipc, system = Value
}
class InstMetaInfo extends Bundle     {
  val fmt = InstFmt()
  val typ = InstType()
}

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
    "b01000".U -> (InstFmt.store, InstType.none),
    "b01101".U -> (InstFmt.upper, InstType.lui),
    "b00101".U -> (InstFmt.upper, InstType.auipc),
    "b11011".U -> (InstFmt.jump, InstType.jal),
    "b11000".U -> (InstFmt.branch, InstType.none)
  ).map { case (key, (fmt, typ)) =>
    key -> {
      val info = Wire(new InstMetaInfo)
      info.fmt := fmt
      info.typ := typ
      info
    }
  }

  io.out := MuxLookup(opcu, 0.U.asTypeOf(new InstMetaInfo()))(lut)
}

class DecodedInstInfo extends InstMetaInfo {
  val imm = Types.UWord
  val rd  = Types.RegAddr
  val rs1 = Types.RegAddr
  val rs2 = Types.RegAddr
}
class DecodedInst     extends Inst         {
  val info = new DecodedInstInfo
}

class IDU extends Module {
  val io = IO(new Bundle {
    val in  = Flipped(Decoupled(new Inst))
    val out = Decoupled(new DecodedInst)
  })

  val recv_fsm = Module(new BusSlaveFSM)
  val send_fsm = Module(new BusMasterFSM)

  recv_fsm.connectMaster(io.in)
  recv_fsm.io.want_recv := io.out.ready

  send_fsm.connectSlave(io.out)
  send_fsm.io.want_send := io.in.valid

  io.out.bits.viewAsSupertype(new Inst) := io.in.bits

  // alias
  val res  = io.out.bits.info
  val inst = io.in.bits.code

  val iinfo_dec = Module(new InstInfoDecoder())
  iinfo_dec.io.opcode                   := io.in.bits.code(6, 0)
  res.viewAsSupertype(new InstMetaInfo) := iinfo_dec.io.out

  res.rd  := inst(11, 7)
  res.rs1 := inst(19, 15)
  res.rs2 := inst(24, 20)

  // fetch IMM
  val immI = Cat(Fill(21, inst(31)), inst(30, 20))
  val immS = Cat(immI(31, 5), inst(11, 8), inst(7))
  val immB = Cat(immI(31, 12), inst(7), immS(10, 1), 0.U(1.W))
  val immU = Cat(inst(31, 12), 0.U(12.W))
  val immJ = Cat(immI(31, 20), inst(19, 12), inst(20), inst(30, 21), 0.U(1.W))

  res.imm := MuxLookup(iinfo_dec.io.out.fmt, "hBAADF00D".U)(
    Seq(
      InstFmt.imm    -> immI,
      InstFmt.jump   -> immJ,
      InstFmt.store  -> immS,
      InstFmt.branch -> immB,
      InstFmt.upper  -> immU
    )
  )

}

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

  io.in.ready  := 0.B
  io.out.valid := 0.B

  // alias
  val inbits = io.in.bits
  val src1   = inbits.src1
  val src2   = inbits.src2

  val s_src1 = src1.asSInt
  val s_src2 = src2.asSInt

  val shamt = src2(4, 0)

  val add_sub_res = Wire(Types.UWord)
  when(inbits.is_imm || inbits.func7t === 0.U) {
    add_sub_res := src1 + src2
  }.elsewhen(inbits.func7t === "b0100000".U) {
    add_sub_res := src1 - src2
  }.otherwise {
    add_sub_res := BADCALL_RESVALUE
    printf("(alu) UNKNOWN func7t %d", inbits.func7t)
  }

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
      3.U -> Mux(src1 < src2, 1.U, 0.U),     // 011: sltu/sltui
      4.U -> (src1 ^ src2),                  // 100: xor/xori
      5.U -> shift_res,                      // 101: srl/srli/sra/srai
      6.U -> (src1 | src2),                  // 110: or/ori
      7.U -> (src1 & src2)                   // 111: and/andi
    )
  )
}

class WriteBackInfo extends Bundle {
  val wen  = Bool()
  val addr = Types.RegAddr
  val data = Types.UWord

  val csr_wen  = Bool()
  val csr_addr = UInt(12.W)
  val csr_data = Types.UWord

  val nxt_pc = Types.UWord
}

class EXU extends Module {
  val io                   = IO(new Bundle {
    val dinst    = Flipped(Decoupled(new DecodedInst))
    val rvec     = Flipped(new RegReadBundle(2))
    val csr_rvec = Flipped(new RegReadBundle(1))
    val out      = Decoupled(new WriteBackInfo)
  })
  val GARBAGE_UNINIT_VALUE = "hDEADBEEF".U

  val alu = Module(new ALU)

  val alu_in = alu.io.in.bits
  val dinst  = io.dinst.bits
  val func3t = dinst.code(14, 12)
  val func7t = dinst.code(31, 25)

  alu_in.is_imm := (dinst.info.fmt === InstFmt.imm)
  alu_in.func3t := func3t
  alu_in.func7t := func7t

  // reg

  io.rvec.en      := true.B
  io.rvec.addr(0) := dinst.info.rs1
  io.rvec.addr(1) := dinst.info.rs2
  val reg_v1 = io.rvec.data(0)
  val reg_v2 = io.rvec.data(1)

  alu_in.src1 := reg_v1
  alu_in.src2 := Mux(dinst.info.fmt === InstFmt.reg, reg_v2, dinst.info.imm)

  // csr

  val is_mret  = dinst.code === "h30200073".U
  val is_ecall = dinst.code === "h73".U

  val csrren    = io.csr_rvec.en
  val csr_addr  = io.csr_rvec.addr(0)
  val csr_rdata = io.csr_rvec.data(0)

  val csrwen    = io.out.bits.csr_wen
  val csr_wdata = io.out.bits.csr_data

  io.out.bits.csr_addr := csr_addr

  object CSROp {
    val _val_beg = 1.U
    val csrrw    = 1.U
    val csrrs    = 2.U
    val _val_end = 3.U
  }

  when(dinst.info.typ === InstType.system) {
    when(is_ecall) {
      csrren   := true.B
      csrwen   := false.B
      csr_addr := CSRAddr.mcause
    }.elsewhen(is_mret) {
      csrren   := true.B
      csrwen   := false.B
      csr_addr := CSRAddr.mepc
    }.otherwise {
      csrren   := MuxLookup(func3t, false.B)(
        Seq(
          CSROp.csrrw -> (dinst.info.rd =/= 0.U),
          CSROp.csrrs -> true.B
        )
      )
      csrwen   := MuxLookup(func3t, false.B)(
        Seq(
          CSROp.csrrw -> true.B,
          CSROp.csrrs -> (reg_v1 =/= 0.U)
        )
      )
      csr_addr := dinst.code(31, 20)
    }
  }.otherwise {
    csrren   := false.B
    csrwen   := false.B
    csr_addr := 0.U
  }

  // wdata

  // for now, system inst, ecall and mret has rd == 0
  // TODO: handle rd != 0 case
  io.out.bits.wen := (dinst.info.rd =/= 0.U) && (dinst.info.typ =/= InstType.none)

  io.out.bits.addr := dinst.info.rd
  io.out.bits.data := MuxLookup(dinst.info.typ, GARBAGE_UNINIT_VALUE)(
    Seq(
      InstType.arithmetic -> alu.io.out.bits,
      InstType.lui        -> dinst.info.imm,
      InstType.auipc      -> (dinst.pc + dinst.info.imm),
      InstType.jalr       -> (dinst.pc + 4.U),
      InstType.jal        -> (dinst.pc + 4.U),
      InstType.load       -> 0.U, // TODO: load from memory
      InstType.system     -> Mux(
        is_ecall || is_mret,
        GARBAGE_UNINIT_VALUE,
        MuxLookup(func3t, GARBAGE_UNINIT_VALUE)(
          Seq(
            CSROp.csrrw -> csr_rdata,
            CSROp.csrrs -> csr_rdata
          )
        )
      )
    )
  )
  when(dinst.info.typ === InstType.system) {
    when(!(is_ecall || is_mret)) {
      when(func3t < CSROp._val_beg || func3t >= CSROp._val_end) {
        printf("(exu) UNKNOWN SYSTEM func3t %d\n", func3t)
      }
    }
  }

  // nxt_pc

  val nxt_pc = io.out.bits.nxt_pc
  val snpc   = dinst.pc + 4.U
  when(is_ecall || is_mret) {
    nxt_pc := csr_rdata
  }.otherwise {
    when(dinst.info.typ === InstType.jalr) {
      nxt_pc := (reg_v1 + dinst.info.imm) &
        Cat(Fill(Types.BitWidth.word - 1, 1.U), 0.U(1.W))
    }.elsewhen(dinst.info.typ === InstType.jal) {
      nxt_pc := dinst.pc + dinst.info.imm
    }.elsewhen(dinst.info.fmt === InstFmt.branch) {
      val take_branch = MuxLookup(func3t, false.B)(
        Seq(
          0.U -> (reg_v1 === reg_v2),              // beq
          1.U -> (reg_v1 =/= reg_v2),              // bne
          4.U -> (reg_v1.asSInt < reg_v2.asSInt),  // blt
          5.U -> (reg_v1.asSInt >= reg_v2.asSInt), // bge
          6.U -> (reg_v1 < reg_v2),                // bltu
          7.U -> (reg_v1 >= reg_v2)                // bgeu
        )
      )
      nxt_pc := Mux(take_branch, dinst.pc + dinst.info.imm, snpc)
    }.otherwise {
      nxt_pc := snpc
    }
  }
}
