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
  val BLOCK_SIZE         = 4
  val BLOCK_NUM          = 16
  val BLOCK_SIZE_INBITS  = BLOCK_SIZE * 8
  val BLOCK_SIZE_INWORDS = BLOCK_SIZE / 4

  val ARLEN = BLOCK_SIZE_INWORDS - 1

  require(BLOCK_SIZE % 4 == 0)
  require(isPow2(BLOCK_SIZE))
  require(isPow2(BLOCK_NUM))

  val BLOCK_INDEX_WIDTH = log2Ceil(BLOCK_NUM)
  val BLOCK_TAG_WIDTH   = 32 - log2Ceil(BLOCK_SIZE) - BLOCK_INDEX_WIDTH

  def extractTag(addr: UInt):        UInt = {
    addr(31, 32 - BLOCK_TAG_WIDTH)
  }
  def extractIndex(addr: UInt):      UInt = {
    addr(32 - BLOCK_TAG_WIDTH - 1, 32 - BLOCK_TAG_WIDTH - BLOCK_INDEX_WIDTH)
  }
  def extractOffset(addr: UInt):     UInt = {
    addr(log2Ceil(BLOCK_SIZE) - 1, 0)
  }
  def extractWordOffset(addr: UInt): UInt = {
    addr(log2Ceil(BLOCK_SIZE) - 1, 2)
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
  // AXI4IO.noShakeConnectAR(io.cpu, io.mem)

  io.mem.araddr  := io.cpu.araddr
  io.mem.arlen   := ICacheParameters.ARLEN.U
  io.mem.arsize  := AXI4IO.SizeType.WORD
  io.mem.arburst := AXI4IO.BurstType.INCR

  io.mem.rready := (state === State.waitMem)

  val nxtCacheData = Wire(UInt(ICacheParameters.BLOCK_SIZE_INBITS.W))
  dontTouch(nxtCacheData)
  nxtCacheData := Cat(io.mem.rdata, rdCacheBlock.data(ICacheParameters.BLOCK_SIZE_INBITS - 1, 32))

  when(state === State.waitMem && io.mem.rvalid) {
    // fill cache
    rdCacheBlock.valid := true.B
    rdCacheBlock.tag   := ICacheParameters.extractTag(rdAddr)
    rdCacheBlock.data  := nxtCacheData
  }

  io.cpu.rvalid := (state === State.respCPU)
  io.cpu.rresp  := AXI4IO.RResp.OKAY
  // TODO: support burst read
  io.cpu.rid    := io.mem.rid
  io.cpu.rlast  := true.B

  // 2^5 = 32
  val hitedData = nxtCacheData >> (ICacheParameters.extractWordOffset(rdAddr) << 5)

  // TODO: impl offset
  // for now assume aligned access
  io.cpu.rdata := hitedData(31, 0)

  state := MuxLookup(state, State.idle)(
    Seq(
      State.idle       -> Mux(io.cpu.arvalid && io.cpu.arready, State.checkCache, State.idle),
      State.checkCache -> Mux(cacheHit, State.respCPU, State.sendFetch),
      State.sendFetch  -> Mux(io.mem.arready, State.waitMem, State.sendFetch),
      State.waitMem    -> Mux(io.mem.rvalid && io.mem.rlast, State.respCPU, State.waitMem),
      State.respCPU    -> Mux(io.cpu.rready, State.idle, State.respCPU)
    )
  )

}
