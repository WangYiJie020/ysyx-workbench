package memory
import chisel3._
import chisel3.util._

// see https://www.chisel-lang.org/api/latest/chisel3/util/circt/dpi/index.html
// chisel has native dpi interface since
//  https://github.com/chipsalliance/chisel/pull/4158
import chisel3.util.circt.dpi._

import common_def._

object AXI4LiteIO {
  val ADDR_WIDTH = Types.BitWidth.word
  val DATA_WIDTH = Types.BitWidth.word

  def AddrT = UInt(ADDR_WIDTH.W)
  def DataT = UInt(DATA_WIDTH.W)

  def StrbT = UInt((DATA_WIDTH / 8).W)

  object RResp{
    val WIDTH = 2
    private def _v(x: Int) = x.U(WIDTH.W)

    val OKAY   = _v(0)
    val EXOKAY = _v(1)
    val SLVERR = _v(2)
    val DECERR = _v(3)
  }
  object BResp{
    val WIDTH = 2
    private def _v(x: Int) = x.U(WIDTH.W)

    val OKAY   = _v(0)
    val EXOKAY = _v(1)
    val SLVERR = _v(2)
    val DECERR = _v(3)
  }

  class _ReqTX extends Bundle {
    // addr read channel
    val ar = Decoupled(AddrT)
    // read data channel
    val r  = Flipped(Decoupled(new Bundle {
      val data = DataT
      val resp = UInt(RResp.WIDTH.W)
    }))

    // addr write channel
    val aw = Decoupled(AddrT)

    // write data channel
    val w = Decoupled(new Bundle {
      val data = DataT
      val strb = StrbT
    })

    // backward channel
    val b = Flipped(Decoupled(UInt(BResp.WIDTH.W)))
  }

  def TX = new _ReqTX()
  def RX = Flipped(TX)
}

class AXI4LiteMemUnit extends Module {
  val io = IO(AXI4LiteIO.RX)

  when(reset.asBool) {
    assert(io.ar.valid === false.B)
    assert(io.aw.valid === false.B)
    assert(io.w.valid === false.B)
  }

  // AR

  val rdAddr = Reg(Types.UWord)

  when(io.ar.valid && io.ar.ready) {
    rdAddr := io.ar.bits
  }

  val sARIdle :: sARWait :: Nil = Enum(2)
  val arState                   = RegInit(sARIdle)
  io.ar.ready := (arState === sARIdle)

  arState := MuxLookup(arState, sARIdle)(
    Seq(
      sARIdle -> Mux(io.ar.valid, sARWait, sARIdle),
      sARWait -> Mux(io.r.valid, sARIdle, sARWait)
    )
  )

  // R

  val rdData = Wire(Types.UWord)

  val sRIdle :: sRWaitMem :: sRWaitRdy :: Nil = Enum(3)
  val rState                                  = RegInit(sRIdle)
  io.r.valid     := (rState === sRWaitRdy)
  io.r.bits.resp := AXI4LiteIO.RResp.OKAY
  io.r.bits.data := rdData

  val memReadFinished = Wire(Bool())
  val memReadPrepared = (arState === sARWait)

  rState := MuxLookup(rState, sRIdle)(
    Seq(
      sRIdle    -> Mux(memReadPrepared, sRWaitMem, sRIdle),
      sRWaitMem -> Mux(memReadFinished, sRWaitRdy, sRWaitMem),
      sRWaitRdy -> Mux(io.r.ready, sRIdle, sRWaitRdy)
    )
  )

  when(rState === sRWaitMem) {
    rdData := RawClockedNonVoidFunctionCall("pmem_read", Types.UWord)(
      clock,
      (!reset.asBool),
      rdAddr
    )
  }

  // for now mem read always finish in one cycle
  memReadFinished := (rState === sRWaitMem)

  // AW

  val wrAddr = Reg(Types.UWord)

  when(io.aw.valid && io.aw.ready) {
    wrAddr := io.aw.bits
  }

  val sAWIdle :: sAWWait :: Nil = Enum(2)
  val awState                   = RegInit(sAWIdle)
  io.aw.ready := (awState === sAWIdle)

  awState := MuxLookup(awState, sAWIdle)(
    Seq(
      sAWIdle -> Mux(io.aw.valid, sAWWait, sAWIdle),
      sAWWait -> Mux(io.b.valid, sAWIdle, sAWWait)
    )
  )

  // W

  val wrData = Reg(Types.UWord)
  val wrMask = Reg(UInt(4.W))
  when(io.w.valid && io.w.ready) {
    wrData := io.w.bits.data
    wrMask := io.w.bits.strb
  }

  val sWIdle :: sWWait :: Nil = Enum(2)
  val wState                  = RegInit(sWIdle)
  io.w.ready := (wState === sWIdle)
  wState     := MuxLookup(wState, sWIdle)(
    Seq(
      sWIdle -> Mux(io.w.valid, sWWait, sWIdle),
      sWWait -> Mux(io.b.valid, sWIdle, sWWait)
    )
  )

  // B

  val sBIdle :: sBWaitAddrOrData :: sBWaitMem :: sBWaitRdy :: Nil = Enum(4)

  val bState = RegInit(sBIdle)
  io.b.valid := (bState === sBWaitRdy)
  io.b.bits  := AXI4LiteIO.BResp.OKAY

  val memWriteFinished = Wire(Bool())
  val memWritePrepared = (awState === sAWWait) && (wState === sWWait)

  bState := MuxLookup(bState, sBIdle)(
    Seq(
      sBIdle           -> Mux(io.aw.valid || io.w.valid, sBWaitAddrOrData, sBIdle),
      sBWaitAddrOrData -> Mux(memWritePrepared, sBWaitMem, sBWaitAddrOrData),
      sBWaitMem        -> Mux(memWriteFinished, sBWaitRdy, sBWaitMem),
      sBWaitRdy        -> Mux(io.b.ready, sBIdle, sBWaitRdy)
    )
  )

  when(bState === sBWaitMem) {
    RawClockedVoidFunctionCall("pmem_write")(
      clock,
      (!reset.asBool),
      wrAddr,
      wrData,
      wrMask.pad(32)
    )
  }
  // for now mem write always finish in one cycle
  memWriteFinished := (bState === sBWaitMem)

}

/*
object MemReqIO {

  // Mem always return 4 bytes at addr & ~3.U
  class _ReadRX extends Bundle {
    val addr = Input(Types.UWord)
    val data = Output(Types.UWord)
    val en   = Input(Bool())

    val respValid = Output(Bool())
  }

  // Mem always write begin at addr & ~3.U 4 bytes
  // Mask bits indicate which byte to write
  class _WriteRX extends Bundle {
    val addr = Input(Types.UWord)
    val data = Input(Types.UWord)
    val mask = Input(UInt(4.W))
    val en   = Input(Bool())

    val done = Output(Bool())
  }

  def ReadRX  = new _ReadRX
  def ReadTX  = Flipped(ReadRX)
  def WriteRX = new _WriteRX
  def WriteTX = Flipped(WriteRX)
}

class MemUnitIO extends Bundle {
  val read  = MemReqIO.ReadRX
  val write = MemReqIO.WriteRX
}
class MemUnit   extends Module {
  val io = IO(new MemUnitIO)

  val sRdIdle :: sRdWait :: Nil = Enum(2)
  val rdState                   = RegInit(sRdIdle)
  rdState := MuxCase(
    rdState,
    Seq(
      (rdState === sRdIdle && io.read.en) -> sRdWait,
      (rdState === sRdWait)               -> sRdIdle
    )
  )

  io.read.respValid := (rdState === sRdWait)

  val enRdCall = io.read.en && (rdState === sRdIdle)

  io.read.data := RawClockedNonVoidFunctionCall("pmem_read", Types.UWord)(
    clock,
    enRdCall && (!reset.asBool),
    io.read.addr
  )

  val sWrIdle :: sWrWait :: Nil = Enum(2)
  val WrState                   = RegInit(sWrIdle)
  WrState := MuxLookup(WrState, sWrIdle)(
    Seq(
      sWrIdle -> Mux(io.write.en, sWrWait, sWrIdle),
      sWrWait -> sWrIdle
    )
  )

  io.write.done := (WrState === sWrWait)
  val enWrCall = io.write.en && (WrState === sWrIdle)

  RawClockedVoidFunctionCall("pmem_write")(
    clock,
    enWrCall && (!reset.asBool),
    io.write.addr,
    io.write.data,
    io.write.mask.pad(32)
  )

}

class LoadStoreFSM extends Module {
  val io = IO(new Bundle {
    val memRd     = MemReqIO.ReadTX
    val memWr     = MemReqIO.WriteTX
    val reqValid  = Input(Bool())
    val addr      = Input(Types.UWord)
    val wen       = Input(Bool())
    val wdata     = Input(Types.UWord)
    val wmask     = Input(UInt(4.W))
    val rdata     = Output(Types.UWord)
    val respValid = Output(Bool())
  })

  val sNoneed :: sWaitMem :: sDone :: Nil = Enum(3)
  val state                               = RegInit(sNoneed)
  state := MuxLookup(state, sNoneed)(
    Seq(
      sNoneed  -> Mux(io.reqValid, sWaitMem, sNoneed),
      sWaitMem -> Mux(io.respValid, sDone, sWaitMem),
      sDone    -> Mux(io.reqValid, sDone, sNoneed)
    )
  )
  val isLoad = !io.wen
  val isStore = io.wen

  io.memRd.en   := (state === sNoneed) && io.reqValid && isLoad
  io.memRd.addr := io.addr

  io.memWr.en   := (state === sNoneed) && io.reqValid && isStore
  io.memWr.addr := io.addr
  io.memWr.data := io.wdata
  io.memWr.mask := io.wmask

  io.rdata     := io.memRd.data
  io.respValid := MuxCase(
    false.B,
    Seq(
      isLoad  -> io.memRd.respValid,
      isStore -> io.memWr.done
    )
  )

}

 */
