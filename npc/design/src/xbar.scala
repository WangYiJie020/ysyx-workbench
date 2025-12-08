package xbar
import chisel3._
import chisel3.util._

import axi4._

class AXI4LiteXBar(mappings: Seq[((UInt, UInt), AXI4LiteIO.Imp)]) extends Module {

  println(s"AXI4LiteXBar mappings: ${mappings.map(_._1)}")
  println(s"AXI4LiteXBar parameter: ${mappings.map(_._2)}")

  require(mappings.nonEmpty, "AXI4LiteXBar requires non-empty mappings.")
  require(mappings.size > 1, "AXI4LiteXBar requires at least two mappings.")
  require(
    mappings.map(_._2).distinct.size == 1,
    "AXI4LiteXBar requires all AXI4LiteIO parameters in mappings to be identical."
  )


  val axiParam = mappings.head._2

  val io = IO(new Bundle {
    val master = AXI4LiteIO.newRX(axiParam.ADDR_WIDTH, axiParam.DATA_WIDTH)
    val slaves = Vec(mappings.size, AXI4LiteIO.newTX(axiParam.ADDR_WIDTH, axiParam.DATA_WIDTH))
  })

  val isAR = Wire(Vec(mappings.size, Bool()))
  val isAW = Wire(Vec(mappings.size, Bool()))

  val hasLastRdReq = RegInit(false.B)
  val hasLastWrReq = RegInit(false.B)
  val lastRdReqIdx = RegInit(0.U(log2Ceil(mappings.size).W))
  val lastWrReqIdx = RegInit(0.U(log2Ceil(mappings.size).W))

  val master = io.master

  for ((((addrBeg, addrEnd), _), i) <- mappings.zipWithIndex) {
    isAR(i) := (master.ar.bits >= addrBeg) && (master.ar.bits < addrEnd)
    isAW(i) := (master.aw.bits >= addrBeg) && (master.aw.bits < addrEnd)

    io.slaves(i).ar.valid := isAR(i) && master.ar.valid
    io.slaves(i).aw.valid := isAW(i) && master.aw.valid

    when(io.slaves(i).ar.valid && io.slaves(i).ar.ready) {
      lastRdReqIdx := i.U
      hasLastRdReq := true.B
    }
    when(io.slaves(i).aw.valid && io.slaves(i).aw.ready) {
      lastWrReqIdx := i.U
      hasLastWrReq := true.B
    }
  }

  master.ar.ready := Mux1H(isAR, io.slaves.map(_.ar.ready))
  master.aw.ready := Mux1H(isAW, io.slaves.map(_.aw.ready))

  master.r.valid := io.slaves(lastRdReqIdx).r.valid && hasLastRdReq
  master.r.bits  := io.slaves(lastRdReqIdx).r.bits
  for(i <- mappings.indices){
    io.slaves(i).r.ready := hasLastRdReq && (i.U === lastRdReqIdx) && master.r.ready
  }

  master.w.ready := io.slaves(lastWrReqIdx).w.ready && hasLastWrReq
  for(i <- mappings.indices){
    io.slaves(i).w.valid := hasLastWrReq && (i.U === lastWrReqIdx) && master.w.valid
  }

  master.b.valid := io.slaves(lastWrReqIdx).b.valid && hasLastWrReq
  master.b.bits  := io.slaves(lastWrReqIdx).b.bits
  for(i <- mappings.indices){
    io.slaves(i).b.ready := hasLastWrReq && (i.U === lastWrReqIdx) && master.b.ready
  }

  when(master.r.valid && master.r.ready) {
    hasLastRdReq := false.B
  }
  when(master.b.valid && master.b.ready) {
    hasLastWrReq := false.B
  }

  io.slaves.foreach { s =>
    s.ar.bits := io.master.ar.bits
    s.aw.bits := io.master.aw.bits
    s.w.bits  := io.master.w.bits
  }

}
