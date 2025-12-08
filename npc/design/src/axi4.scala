package axi4
import chisel3._
import chisel3.util._

object AXI4LiteIO {

  object RResp {
    val WIDTH = 2
    private def _v(x: Int) = x.U(WIDTH.W)

    val OKAY   = _v(0)
    val EXOKAY = _v(1)
    val SLVERR = _v(2)
    val DECERR = _v(3)
  }
  object BResp {
    val WIDTH = 2
    private def _v(x: Int) = x.U(WIDTH.W)

    val OKAY   = _v(0)
    val EXOKAY = _v(1)
    val SLVERR = _v(2)
    val DECERR = _v(3)
  }

  class Imp(val ADDR_WIDTH:Int,val DATA_WIDTH:Int) extends Bundle {

    def AddrT = UInt(ADDR_WIDTH.W)
    def DataT = UInt(DATA_WIDTH.W)

    def StrbT = UInt((DATA_WIDTH / 8).W)

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

  def newTX(addrWidth:Int=32,dataWidth:Int=32) = new Imp(addrWidth,dataWidth)
  def newRX(addrWidth:Int=32,dataWidth:Int=32) = Flipped(newTX(addrWidth,dataWidth))

  // for default
  def TX = newTX()
  def RX = newRX()
}
