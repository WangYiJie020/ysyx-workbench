package cpu

import chisel3._
import chisel3.util.{Decoupled, Enum}
import chisel3.util.MuxLookup

class Inst extends Bundle {
  val code = Output(UInt(32.W))
  val pc   = Output(UInt(32.W))
}

class IFU extends Module {
  val io                            = IO(new Bundle { val out = Decoupled(new Inst) })
  val s_idle :: s_wait_ready :: Nil = Enum(2)
  val state                         = RegInit(s_idle)
  state        := MuxLookup(state, s_idle)(
    List(
      s_idle       -> Mux(io.out.valid, s_wait_ready, s_idle),
      s_wait_ready -> Mux(io.out.ready, s_idle, s_wait_ready)
    )
  )
  io.out.valid := 0.B
}

object InstFmt     extends ChiselEnum {
  val imm, reg, store, upper, jump, branch = Value
}
object InstType    extends ChiselEnum {
  val none, arithmetic, load, jalr, lui, auipc, system = Value
}
class InstMetaInfo extends Bundle     {
  val fmt = InstFmt()
  val typ = InstType()
}

class IInfoDecoder extends Module {
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
    "b01100".U -> (InstFmt.reg, InstType.none),
    "b01000".U -> (InstFmt.store, InstType.none),
    "b01101".U -> (InstFmt.upper, InstType.lui),
    "b00101".U -> (InstFmt.upper, InstType.auipc),
    "b11011".U -> (InstFmt.jump, InstType.none),
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

class DecodedInst(reg_addr_width:Int=5) extends InstMetaInfo{
  val imm=UInt(32.W)
  val rd=UInt(reg_addr_width.W)
  val rs1=UInt(reg_addr_width.W)
  val rs2=UInt(reg_addr_width.W)
}

class IDU extends Module {
  val io = IO(new Bundle {
    val in  = Flipped(Decoupled(new Inst))
    val out = Output(new DecodedInst())

  })

  io.in.ready := 0.B

  val iinfo_dec = Module(new IInfoDecoder())
  iinfo_dec.io.opcode := io.in.bits.code(6, 0)
  io.out              := iinfo_dec.io.out
  io.out.rs2:=0.U

}
