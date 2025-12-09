package axi4
import chisel3._
import chisel3.util._

object AXI4IO {

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

  // Master interface
  class Imp(val ADDR_WIDTH: Int, val DATA_WIDTH: Int) extends Bundle {

    def AddrT = UInt(ADDR_WIDTH.W)
    def DataT = UInt(DATA_WIDTH.W)

    def StrbT = UInt((DATA_WIDTH / 8).W)

    // aw channel

    val awready = Input(Bool())
    val awvalid = Output(Bool())
    val awaddr  = Output(AddrT)
    val awid    = Output(UInt(4.W))
    val awlen   = Output(UInt(8.W))
    val awsize  = Output(UInt(3.W))
    val awburst = Output(UInt(2.W))

    // w channel

    val wready = Input(Bool())
    val wvalid = Output(Bool())
    val wdata  = Output(DataT)
    val wstrb  = Output(StrbT)
    val wlast  = Output(Bool())

    // b channel

    val bready = Output(Bool())
    val bvalid = Input(Bool())
    val bresp  = Input(UInt(BResp.WIDTH.W))
    val bid    = Input(UInt(4.W))

    // ar channel

    val arready = Input(Bool())
    val arvalid = Output(Bool())
    val araddr  = Output(AddrT)
    val arid    = Output(UInt(4.W))
    val arlen   = Output(UInt(8.W))
    val arsize  = Output(UInt(3.W))
    val arburst = Output(UInt(2.W))

    // r channel

    val rready = Output(Bool())
    val rvalid = Input(Bool())
    val rdata  = Input(DataT)
    val rresp  = Input(UInt(RResp.WIDTH.W))
    val rlast  = Input(Bool())
    val rid    = Input(UInt(4.W))
  }

  def noShakeConnectAW(master: Imp, slave: Imp) = {
    slave.awaddr  := master.awaddr
    slave.awid    := master.awid
    slave.awlen   := master.awlen
    slave.awsize  := master.awsize
    slave.awburst := master.awburst
  }
  def noShakeConnectW(master: Imp, slave: Imp)  = {
    slave.wdata := master.wdata
    slave.wstrb := master.wstrb
    slave.wlast := master.wlast
  }
  def noShakeConnectB(master: Imp, slave: Imp)  = {
    master.bresp := slave.bresp
    master.bid   := slave.bid
  }

  def connectAW(master: Imp, slave: Imp) = {
    noShakeConnectAW(master, slave)
    slave.awvalid  := master.awvalid
    master.awready := slave.awready
  }
  def connectW(master: Imp, slave: Imp)  = {
    noShakeConnectW(master, slave)
    slave.wvalid  := master.wvalid
    master.wready := slave.wready
  }
  def connectB(master: Imp, slave: Imp)  = {
    noShakeConnectB(master, slave)
    master.bvalid := slave.bvalid
    slave.bready  := master.bready
  }

  def noShakeConnectAR(master: Imp, slave: Imp) = {
    slave.araddr  := master.araddr
    slave.arid    := master.arid
    slave.arlen   := master.arlen
    slave.arsize  := master.arsize
    slave.arburst := master.arburst
  }

  def noShakeConnectR(master: Imp, slave: Imp) = {
    master.rdata := slave.rdata
    master.rresp := slave.rresp
    master.rlast := slave.rlast
    master.rid   := slave.rid
  }

  def newMaster(addrWidth: Int = 32, dataWidth: Int = 32) = new Imp(addrWidth, dataWidth)
  def newSlave(addrWidth:  Int = 32, dataWidth: Int = 32) = Flipped(newMaster(addrWidth, dataWidth))

  class MasterT extends Bundle {
    val master = newMaster()
    def ioImp  = master

    def dontCareAW() = {
      master.awvalid := false.B
      master.awaddr  := 0.U
      master.awid    := 0.U
      master.awlen   := 0.U
      master.awsize  := 0.U
      master.awburst := 0.U
    }
    def dontCareW()  = {
      master.wvalid := false.B
      master.wdata  := 0.U
      master.wstrb  := 0.U
      master.wlast  := false.B
    }
    def dontCareB()  = {
      master.bready := false.B
    }
    def dontCareNonLiteAR() = {
      master.arid    := 0.U
      master.arlen   := 0.U
      master.arsize  := 0.U
      master.arburst := 0.U
    }
    def dontCareNonLiteAW() = {
      master.awid    := 0.U
      master.awlen   := 0.U
      master.awsize  := 0.U
      master.awburst := 0.U
    }

    def dontCareAR() = {
      master.arvalid := false.B
      master.araddr  := 0.U
      master.arid    := 0.U
      master.arlen   := 0.U
      master.arsize  := 0.U
      master.arburst := 0.U
    }
    def dontCareR()  = {
      master.rready := false.B
    }

  }
  class SlaveT extends Bundle {
    val slave = newSlave()
    def ioImp = slave

    def dontCareAW() = {
      slave.awready := false.B
    }
    def dontCareW()  = {
      slave.wready := false.B
    }
    def dontCareB()  = {
      slave.bvalid := false.B
      slave.bresp  := 0.U
      slave.bid    := 0.U
    }
    def dontCareAR() = {
      slave.arready := false.B
    }
    def dontCareR()  = {
      slave.rvalid := false.B
      slave.rdata  := 0.U
      slave.rresp  := 0.U
      slave.rlast  := false.B
      slave.rid    := 0.U
    }

    def dontCareNonLiteR() = {
      slave.rid    := 0.U
      slave.rlast  := false.B
    }
    def dontCareNonLiteB() = {
      slave.bid    := 0.U
    }
  }

  def Master = new MasterT
  def Slave  = new SlaveT

  def connectMasterSlave(master: MasterT, slave: SlaveT) = {
    master.ioImp <> slave.ioImp
  }
}

object AXI4LiteIO_ {

  def BResp = AXI4IO.BResp
  def RResp = AXI4IO.RResp

  class Imp(val ADDR_WIDTH: Int, val DATA_WIDTH: Int) extends Bundle {

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

  def newTX(addrWidth: Int = 32, dataWidth: Int = 32) = new Imp(addrWidth, dataWidth)
  def newRX(addrWidth: Int = 32, dataWidth: Int = 32) = Flipped(newTX(addrWidth, dataWidth))

  // for default
  def TX = newTX()
  def RX = newRX()
}
