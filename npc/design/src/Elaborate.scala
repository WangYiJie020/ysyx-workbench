import scala.sys.process._

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
  println("Emitting cpu Verilog...")
  circt.stage.ChiselStage.emitSystemVerilogFile(new top.ysyx_25100261(), args, firtoolOptions)
  println("Finish emit cpu Verilog.")

  println("Preprocessing cpu Verilog...")
  val designName = "ysyx_25100261"
  val preProcCore = s"./scripts/preproc_vsrcs.sh ${args(1)} ${designName}".!
  if (preProcCore != 0) sys.exit(preProcCore)

  circt.stage.ChiselStage.emitSystemVerilogFile(new top.NPCTestSoC(),
    Array("--target-dir", "build/npctestsoc"),
    firtoolOptions
  )

  println("Preprocessing SoC Verilog...")
  val preProcSoC = s"./scripts/preproc_vsrcs.sh build/npctestsoc ${designName}".!
  if (preProcSoC != 0) sys.exit(preProcSoC)
}
