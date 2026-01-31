package icache

import chisel3._
import chisel3.util._

import common_def._
import axi4._

class ICacheIO extends Bundle {
  val cpu = AXI4IO.Slave
  val mem = AXI4IO.Master
}

object ICacheParameters {
  val BLOCK_SIZE        = 4
  val BLOCK_NUM         = 16
  val BLOCK_SIZE_INBITS = BLOCK_SIZE * 8

  require(BLOCK_SIZE % 4 == 0)
  require(isPow2(BLOCK_SIZE))
  require(isPow2(BLOCK_NUM))

  val BLOCK_INDEX_WIDTH = log2Ceil(BLOCK_NUM)
  val BLOCK_TAG_WIDTH   = 32 - log2Ceil(BLOCK_SIZE) - BLOCK_INDEX_WIDTH

  def extractTag(addr: UInt):    UInt = {
    addr(31, 32 - BLOCK_TAG_WIDTH)
  }
  def extractIndex(addr: UInt):  UInt = {
    addr(32 - BLOCK_TAG_WIDTH - 1, 32 - BLOCK_TAG_WIDTH - BLOCK_INDEX_WIDTH)
  }
  def extractOffset(addr: UInt): UInt = {
    addr(log2Ceil(BLOCK_SIZE) - 1, 0)
  }
}

class ICacheBlock extends Bundle {
  val valid = Bool()
  val tag   = UInt(ICacheParameters.BLOCK_TAG_WIDTH.W)
  val data  = UInt(ICacheParameters.BLOCK_SIZE_INBITS.W)

  // Assume index and offset are correct
  def matchAddr(addr: UInt): Bool = {
    tag === ICacheParameters.extractTag(addr) && valid
  }
}

class ICache extends Module {
  val io = IO(new ICacheIO)

  io.cpu.dontCareAW()
  io.cpu.dontCareW()
  io.cpu.dontCareB()

  io.mem.dontCareAW()
  io.mem.dontCareW()
  io.mem.dontCareB()


  val blocks = RegInit(VecInit(Seq.fill(ICacheParameters.BLOCK_NUM)(0.U.asTypeOf(new ICacheBlock))))

  object State extends ChiselEnum {
    val idle, checkCache, sendFetch, waitMem, respCPU = Value
  }
  val state = RegInit(State.idle)

  val rdAddr = Reg(Types.UWord)

  when(io.cpu.arvalid && io.cpu.arready) {
    rdAddr := io.cpu.araddr
  }

  val rdIdx        = ICacheParameters.extractIndex(rdAddr)
  val rdCacheBlock = blocks(rdIdx)
  val cacheHit     = rdCacheBlock.matchAddr(rdAddr)

  io.cpu.arready := (state === State.idle) && (!reset.asBool)
  io.mem.arvalid := (state === State.sendFetch)
  AXI4IO.noShakeConnectAR(io.cpu, io.mem)

  io.mem.rready := (state === State.waitMem)

  when(state === State.waitMem && io.mem.rvalid) {
    // fill cache
    rdCacheBlock.valid := true.B
    rdCacheBlock.tag   := ICacheParameters.extractTag(rdAddr)
    rdCacheBlock.data  := io.mem.rdata
  }

  io.cpu.rvalid  := (state === State.respCPU)
  io.cpu.rresp   := AXI4IO.RResp.OKAY
  // TODO: support burst read
  io.cpu.rid     := 0.U
  io.cpu.rlast   := true.B

  // TODO: impl offset
  // for now assume aligned access
  io.cpu.rdata   := Mux(cacheHit, rdCacheBlock.data, io.mem.rdata)

  state := MuxLookup(state, State.idle)(
    Seq(
      State.idle        -> Mux(io.cpu.arvalid && io.cpu.arready, State.checkCache, State.idle),
      State.checkCache  -> Mux(cacheHit, State.respCPU, State.sendFetch),
      State.sendFetch   -> Mux(io.mem.arready, State.waitMem, State.sendFetch),
      State.waitMem     -> Mux(io.mem.rvalid, State.respCPU, State.waitMem),
      State.respCPU     -> Mux(io.cpu.rready, State.idle, State.respCPU)
    )
  )

}
