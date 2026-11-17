import config.{CpuConfig, CpuIsa}
import scala.sys.process._

object Elaborate extends App {
  final case class Options(
      targetDir: String = "",
      isa: String = "",
      platform: String = "",
      resetPc: BigInt = 0
  )

  def parse(rest: List[String], options: Options = Options()): Options = rest match {
    case "--target-dir" :: value :: tail => parse(tail, options.copy(targetDir = value))
    case "--isa" :: value :: tail        => parse(tail, options.copy(isa = value))
    case "--platform" :: value :: tail   => parse(tail, options.copy(platform = value))
    case "--reset-pc" :: value :: tail =>
      parse(tail, options.copy(resetPc = BigInt(value.stripPrefix("0x"), 16)))
    case Nil   => options
    case other => throw new IllegalArgumentException(s"invalid arguments: ${other.mkString(" ")}")
  }

  val options = parse(args.toList)
  require(options.targetDir.nonEmpty, "--target-dir is required")
  require(
    Set("npc", "ysyxsoc", "core", "iverilog", "de").contains(options.platform),
    s"unsupported platform: ${options.platform}"
  )
  val cpuConfig = CpuConfig(
    CpuIsa.parse(options.isa),
    options.resetPc,
    enableDpi = options.platform == "npc" || options.platform == "ysyxsoc",
    enableIcarus = options.platform == "iverilog",
    enableNpcTrap = options.platform == "de"
  )

  val firtoolOptions = Array(
    "--lowering-options=disallowLocalVariables,disallowPackedArrays,locationInfoStyle=wrapInAtSquareBracket",
    "-disable-all-randomization",
    "-strip-debug-info"
  )

  val emitDir = s"${options.targetDir}/${options.isa}"
  val generator = if (Set("ysyxsoc", "core", "de").contains(options.platform)) {
    () => new top.ysyx_25100261(cpuConfig)
  } else {
    () => new standalone.TestSoC(cpuConfig)
  }

  println(f"Emitting ${options.isa}/${options.platform}, reset PC 0x${options.resetPc}%08x to $emitDir")
  circt.stage.ChiselStage.emitSystemVerilogFile(
    generator(),
    Array("--target-dir", emitDir),
    firtoolOptions
  )

  val preProc = s"./scripts/preproc_vsrcs.sh $emitDir ysyx_25100261".!
  if (preProc != 0) sys.exit(preProc)
}
