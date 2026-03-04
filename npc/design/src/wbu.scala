package cpu

import chisel3._
import chisel3.util.{Cat, Decoupled, DecoupledIO, Enum, Fill, MuxLookup}

import chisel3.experimental.dataview._

import regfile._
import common_def._
import dpiwrap._
import busfsm._

import chisel3.util.circt.dpi._

class WriteBackInfo extends Bundle {
  val gpr = GPRegReqIO.TX.Write

  val csr           = CSRegReqIO.TX.Write
  val csr_ecallflag = Bool()

  val is_ebreak = Bool()

  val skipDifftest = Bool()

  val pc     = Types.UWord
  val nxt_pc = Types.UWord

  val iid = Types.InstID
}

object ExtractFwdInfoFromWrBack {
  def apply(info: DecoupledIO[WriteBackInfo]): WrBackForwardInfo = {
    val wrBack = info.bits

    val out = Wire(new WrBackForwardInfo)
    out.addr      := wrBack.gpr.addr
    out.enWr      := wrBack.gpr.en && info.valid
    out.dataVaild := info.valid
    out.data      := wrBack.gpr.data
    out
  }
}

class WBU extends Module {
  val io = IO(new Bundle {
    val in       = Flipped(Decoupled(new WriteBackInfo))
    val gpr      = GPRegReqIO.TX.Write
    val csr      = CSRegReqIO.TX.Write
    val is_ecall = Output(Bool())
    val done     = Output(Bool())
  })

  val wbinfo = io.in.bits
  val valid  = io.in.valid

  io.in.ready := true.B

  io.gpr.en   := wbinfo.gpr.en && valid
  io.gpr.addr := wbinfo.gpr.addr
  io.gpr.data := wbinfo.gpr.data

  io.csr.en   := wbinfo.csr.en && valid
  io.csr.addr := wbinfo.csr.addr
  io.csr.data := wbinfo.csr.data
  io.is_ecall := wbinfo.csr_ecallflag && valid

  io.done := valid

  dontTouch(io)
}

class DifftestWriteBackInfo extends Bundle {
  val pc= Types.UWord
  val nxtPC = Types.UWord
  val isEBreak = Bool()
  val needSkipRef = Bool()
}
class WBUForDifftest extends Module {
  val io = IO(new Bundle {
    val in = Flipped(Decoupled(new DifftestWriteBackInfo))
  })
  val wbinfo = io.in.bits
  val valid  = io.in.valid
  io.in.ready := true.B

  val isEBreak = WireDefault(wbinfo.isEBreak && valid)
  dontTouch(isEBreak)
  when(isEBreak) {
    ClockedCallVoidDPIC("raise_ebreak")(clock, isEBreak)
    stop()
  }

  when(valid && wbinfo.needSkipRef) {
    ClockedCallVoidDPIC("skip_difftest_ref")(clock, valid && wbinfo.needSkipRef, wbinfo.pc, wbinfo.nxtPC)
  }

  when(valid && (!isEBreak)) {
    ClockedCallVoidDPIC("pc_upd")(clock, valid && !isEBreak, wbinfo.pc, wbinfo.nxtPC)
  }
}
