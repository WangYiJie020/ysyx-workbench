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

  val LED = ("h80200040".U, "h80200044".U)

  object SelfExtSpace {
    val UART = ("h10000000".U, "h10001000".U)
  }
}

class LED extends Module {
  val io = IO(AXI4IO.Slave)
  io := DontCare

  val data = RegInit(0.U(32.W))
  val sio = io

  sio.awready := true.B
  sio.wready  := true.B

  sio.bvalid := true.B
  sio.bresp  := AXI4IO.BResp.OKAY

  when(sio.wvalid) {
    data := sio.wdata
    printf(cf"LED <- $data%u\n")
  }

}

class JYDDevices extends Module with TestSoCDevice {
  val io = IO(AXI4IO.Slave)
  io := DontCare

  val uart = Module(new UARTUnit)
  val irom = Module(new AXI4MemUnit(1024 * 16))
  val dram = Module(new AXI4MemUnit(1024 * 256))

  val led = Module(new LED)

  io <> AXI4XBar(
    Seq(
      AddrSpace.IROM -> irom.io,
      AddrSpace.DRAM -> dram.io,

      AddrSpace.LED -> led.io,

      AddrSpace.SelfExtSpace.UART -> uart.io
    )
  )
}
