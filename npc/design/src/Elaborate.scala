import scala.sys.process._

import testSoC._
import common_def.CPUParameters

object Elaborate extends App {
  if (args.size != 2) {
    println("Usage: --target-dir <dir>")
    sys.exit(1)
  }
  val firtoolOptions = Array(
    "--lowering-options=" + List(
      // make yosys happy
      // see https://github.com/llvm/circt/blob/main/docs/VerilogGeneration.md
      "disallowLocalVariables",
      "disallowPackedArrays",
      "locationInfoStyle=wrapInAtSquareBracket"
    ).reduce(_ + "," + _),
    "-disable-all-randomization",
    "-strip-debug-info"
  )

  val designName = "ysyx_25100261"

  def emit(gen: => chisel3.RawModule, emitDir: String) = {
    println(s"Emitting Verilog... to $emitDir")
    circt.stage.ChiselStage.emitSystemVerilogFile(
      gen,
      Array("--target-dir", emitDir),
      firtoolOptions
    )

    println(s"Preprocessing Verilog... on $emitDir")
    val preProcSoC = s"./scripts/preproc_vsrcs.sh ${emitDir} ${designName}".!
    if (preProcSoC != 0) sys.exit(preProcSoC)
    println(s"Finish emitting and preprocessing Verilog on $emitDir")
  }

  emit(new top.ysyx_25100261(CPUParameters(gprAddrWidth = 4)), args(1))
  emit(new top.ysyx_25100261(CPUParameters(gprAddrWidth = 5)), s"${args(1)}/riscv32i")

  emit(new TestSoC(new npc.NPCDevices), "build/testsoc/npc")
  emit(new TestSoC(new jyd.JYDDevices), "build/testsoc/jyd")
}
