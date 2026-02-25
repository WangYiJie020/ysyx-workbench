package dpiwrap

import chisel3._
import chisel3.util._

import chisel3.layer._

import chisel3.util.circt.dpi._

object DPICLayer extends Layer(LayerConfig.Inline)

object ClockedCallVoidDPIC {
  def apply(
    name:       String,
    inputNames: Option[Seq[String]] = None
  )(clock:      Clock,
    enable:     Bool,
    args:       Data*
  ): Unit = {
    block(DPICLayer) {
      println(s"use ${name.toUpperCase}")
      RawClockedVoidFunctionCall(name)(
        clock,
        enable,
        args: _*
      )
    }
  }
}

object SkipDifftestRef {
  def apply(clock: Clock, enable: Bool): Unit = {
    ClockedCallVoidDPIC("skip_difftest_ref")(clock, enable)
  }
}
