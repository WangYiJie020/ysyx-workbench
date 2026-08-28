package config

sealed trait CpuIsa

object CpuIsa {
  case object MiniRV extends CpuIsa
  case object RV32E extends CpuIsa

  def parse(name: String): CpuIsa = name match {
    case "minirv"   => MiniRV
    case "riscv32e" => RV32E
    case other       => throw new IllegalArgumentException(s"unsupported ISA: $other")
  }
}

final case class CpuConfig(isa: CpuIsa, resetPc: BigInt, enableDpi: Boolean = true)
