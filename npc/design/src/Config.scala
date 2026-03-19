package config

import chisel3._

object Config {
  // not affect area, yosys can optimize it away
  val genStageLog: Boolean = false
  val useBTBAndBP: Boolean = false
}
