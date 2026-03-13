package jyd

import chisel3._
import chisel3.util._

import axi4._
import xbar._
import uart._

import top.{CPUCoreAsBlackBox, PCProviderAsBlackBox}
import testSoC._

object AddrSpace {
  val IROM = ("h80000000".U, "h80004000".U)
  val DRAM = ("h80100000".U, "h81400000".U)
  val MMIO = ("h80200000".U, "h80200100".U)

  object SelfExtSpace {
    val UART = ("h10000000".U, "h10001000".U)
  }
}

class JYDDevices extends Module with TestSoCDevice {
  val io = IO(AXI4IO.Slave)
  io := DontCare

  val uart = Module(new UARTUnit)
  val irom = Module(new AXI4MemUnit(1024 * 16))
  val dram = Module(new AXI4MemUnit(1024 * 256))

  io <> AXI4XBar(
    Seq(
      AddrSpace.IROM -> irom.io,
      AddrSpace.DRAM -> dram.io,

      AddrSpace.SelfExtSpace.UART -> uart.io
    )
  )
}
