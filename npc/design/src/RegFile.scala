package regfile

import chisel3._
import chisel3.util.Counter
import cpu.Types
import cpu.Types.Ops._
import chisel3.util.MuxLookup

class RegReadBundle(N: Int) extends Bundle {
  require((1 << Types.BitWidth.reg_addr) >= N)
  val en   = Input(Bool())
  val addr = Input(Vec(N, Types.RegAddr))
  val data = Output(Vec(N, Types.RegAddr))
}
class RegWriteBundle        extends Bundle {
  val en   = Input(Bool())
  val addr = Input(Types.RegAddr)
  val data = Input(Types.UWord)
}

class RegisterFile(READ_PORTS: Int = 2) extends Module {
  val N_REG = 1 << Types.BitWidth.reg_addr

  val io  = IO(new Bundle {
    val write = new RegWriteBundle()
    val rvec  = new RegReadBundle(READ_PORTS)
  })
  val reg = RegInit(VecInit(Seq.fill(N_REG)(0.UWord)))

  when(io.write.en) {
    reg(io.write.addr) := io.write.data
  }
  for (i <- 0 until READ_PORTS) {
    when(io.rvec.addr(i) === 0.U) {
      io.rvec.data(i) := 0.U
    }.otherwise {
      io.rvec.data(i) := reg(io.rvec.addr(i))
    }
  }
}

object CSRAddr {
  val mstatus = "h300".U(12.W)
  val mtvec   = "h305".U(12.W)
  val mepc    = "h341".U(12.W)
  val mcause  = "h342".U(12.W)
  val mcycle  = "hB00".U(12.W)
  val mcycleh = "hB80".U(12.W)
}

class ControlStatusRegisterFile extends Module {
  val io = IO(new Bundle {
    val addr     = Input(UInt(12.W))
    val wdata    = Input(Types.UWord)
    val wen      = Input(Bool())
    val is_ecall = Input(Bool())
    val ren      = Input(Bool())
    val rdata    = Output(Types.UWord)
  })

  val mcycle64 = RegInit(0.U(64.W))
  mcycle64 := mcycle64 + 1.U

  val mvendor_id = "h79737978".U(32.W) // ysyx
  val march_id   = "d25100261".U(32.W)

  val mstatus = RegInit("h00001800".U(32.W))
  val mepc    = RegInit(0.U(32.W))
  val mcause  = RegInit(0.U(32.W))
  val mtvec   = RegInit(0.U(32.W))

  when(io.ren) {
    io.rdata := MuxLookup(io.addr, 0.U)(
      Seq(
        CSRAddr.mcycle  -> mcycle64(31, 0),  
        CSRAddr.mcycleh -> mcycle64(63, 32), 
        CSRAddr.mstatus -> mstatus,
        CSRAddr.mepc    -> mepc,
        CSRAddr.mcause  -> mcause,
        CSRAddr.mtvec   -> mtvec
      )
    )

  }.otherwise {
    io.rdata := 0.U
  }
}
