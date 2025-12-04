package common_def

import chisel3._
import chisel3.util._

object Types {
  object BitWidth {
    val reg_addr = 5
    val csr_addr = 12
    val word     = 32
  }
  def UWord = UInt(BitWidth.word.W)
  def RegAddr = UInt(BitWidth.reg_addr.W)

  object Ops {
    implicit class StringOps(val s: String) extends AnyVal {
      def UWord = s.U(BitWidth.word.W)
    }
    implicit class IntOps(val s: Int)       extends AnyVal {
      def UWord = s.U(BitWidth.word.W)
    }
  }
}
import Types.Ops._


