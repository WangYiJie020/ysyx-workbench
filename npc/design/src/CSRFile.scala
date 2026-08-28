package core

import chisel3._
import chisel3.util._

object CSRAddress {
  val mstatus = "h300".U(12.W)
  val mtvec = "h305".U(12.W)
  val mepc = "h341".U(12.W)
  val mcause = "h342".U(12.W)
  val mcycle = "hb00".U(12.W)
  val mcycleh = "hb80".U(12.W)
  val mvendorid = "hf11".U(12.W)
  val marchid = "hf12".U(12.W)
}

class CSRFileIO extends Bundle {
  val readAddr = Input(UInt(12.W))
  val readData = Output(UInt(32.W))
  val writeEn = Input(Bool())
  val writeAddr = Input(UInt(12.W))
  val writeData = Input(UInt(32.W))
  val trapEn = Input(Bool())
  val trapPc = Input(UInt(32.W))
  val mtvec = Output(UInt(32.W))
  val mepc = Output(UInt(32.W))
}

class CSRFile extends Module {
  val io = IO(new CSRFileIO)

  private val mstatus = RegInit("h00001800".U(32.W))
  private val mtvec = RegInit(0.U(32.W))
  private val mepc = RegInit(0.U(32.W))
  private val mcause = RegInit(0.U(32.W))
  private val cycle = RegInit(0.U(64.W))

  cycle := cycle + 1.U

  io.readData := MuxLookup(io.readAddr, 0.U)(Seq(
    CSRAddress.mstatus -> mstatus,
    CSRAddress.mtvec -> mtvec,
    CSRAddress.mepc -> mepc,
    CSRAddress.mcause -> mcause,
    CSRAddress.mcycle -> cycle(31, 0),
    CSRAddress.mcycleh -> cycle(63, 32),
    CSRAddress.mvendorid -> "h79737978".U,
    CSRAddress.marchid -> 25100261.U
  ))

  when(io.writeEn) {
    switch(io.writeAddr) {
      is(CSRAddress.mstatus) { mstatus := io.writeData }
      is(CSRAddress.mtvec) { mtvec := io.writeData }
      is(CSRAddress.mepc) { mepc := io.writeData }
      is(CSRAddress.mcause) { mcause := io.writeData }
      is(CSRAddress.mcycle) { cycle := Cat(cycle(63, 32), io.writeData) }
      is(CSRAddress.mcycleh) { cycle := Cat(io.writeData, cycle(31, 0)) }
    }
  }

  when(io.trapEn) {
    mepc := io.trapPc
    mcause := 11.U
  }

  io.mtvec := mtvec
  io.mepc := mepc
}
