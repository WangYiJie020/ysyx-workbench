Chisel Project Template
=======================

Another version of the [Chisel template](https://github.com/ucb-bar/chisel-template) supporting mill.
mill is another Scala/Java build tool without obscure DSL like SBT. It is much faster than SBT.

Contents at a glance:

* `.gitignore` - helps Git ignore junk like generated files, build products, and temporary files.
* `build.mill` - instructs mill to build the Chisel project
* `Makefile` - rules to call mill
* `playground/src/GCD.scala` - GCD source file
* `playground/src/DecoupledGCD.scala` - another GCD source file
* `playground/src/Elaborate.scala` - wrapper file to call chisel command with the GCD module
* `playground/test/src/GCDSpec.scala` - GCD tester

Feel free to rename or delete files under `playground/` or use them as a reference/template.

## Getting Started

First, install mill by referring to the documentation [here](https://com-lihaoyi.github.io/mill).

To run all tests in this design (recommended for test-driven development):
```bash
make test
```

To generate Verilog:
```bash
make verilog
```

To synthesize both CPU-only ISA variants with ECC and ICsprout55 at 400 MHz:
```bash
make ecc-all
```

The first run downloads ECC, OSS CAD Suite, and the required ICsprout55 Liberty
files into `build/tools`; it does not install them globally. Use `ecc-minirv` or
`ecc-riscv32e` for one variant. `ECC_FREQ_MHZ` and `ECC_SYNTH_STRATEGY` override
the default frequency and Yosys strategy.
