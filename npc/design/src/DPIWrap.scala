package dpiwrap

import chisel3._
import chisel3.util._

import chisel3.layer._

import chisel3.util.circt.dpi._

import scala.collection.mutable

import chisel3.probe.{Probe, ProbeValue}

object DPICLayer extends Layer(LayerConfig.Inline)

object DPICUseSummary {
  val usedDPICs:         mutable.Set[String] = mutable.Set.empty
  def add(name: String): Unit                = {
    usedDPICs += name
  }

}

object ClockedCallVoidDPIC {
  def apply(
    name:       String,
    inputNames: Option[Seq[String]] = None
  )(clock:      Clock,
    enable:     Bool,
    args:       Data*
  ): Unit = {
    block(DPICLayer) {
      DPICUseSummary.add(name)
      RawClockedVoidFunctionCall(name)(
        clock,
        enable,
        args: _*
      )
    }
  }
}

class DPICModuleWrapper[T <: Data](
  funcName:   String,
  retType:    T, // retType need to be a instance of Data
  argTypes:   Seq[Data],
  inputNames: Option[Seq[String]] = None,
  outputName: Option[String] = None)
    extends Module {

  val io = IO(new Bundle {
    val en   = Input(Bool())
    val args = Input(MixedVec(argTypes.map(_.cloneType)))
  })

  val retData = IO(Output(Probe(retType, DPICLayer)))

  val wire_p = Wire(Probe(retType, DPICLayer))
  layer.block(DPICLayer) {
    DPICUseSummary.add(name)
    val dpicRetVal = RawUnclockedNonVoidFunctionCall(
      funcName,
      retType,
      inputNames,
      outputName
    )(io.en, io.args: _*)
    dontTouch(dpicRetVal)
    probe.define(wire_p, probe.ProbeValue(dpicRetVal))
  }
  dontTouch(wire_p)
  probe.define(retData, wire_p)
}

object UnclockedCallNonVoidDPIC {
  def apply[T <: Data](
    name:       String,
    ret:        => T,
    inputNames: Option[Seq[String]] = None,
    outputName: Option[String] = None
  )(enable:     Bool,
    args:       Data*
  ) = {
    layer.enable(DPICLayer)
    val wrapper = Module(new DPICModuleWrapper(name, ret, args, inputNames, outputName))
    wrapper.io.en := enable
    wrapper.io.args := VecInit(args)
    probe.read(wrapper.retData)
  }
}

object SkipDifftestRef {
  def apply(clock: Clock, enable: Bool): Unit = {
    ClockedCallVoidDPIC("skip_difftest_ref")(clock, enable)
  }
}
