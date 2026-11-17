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

To run a CPU-only AM image with Icarus Verilog four-state simulation:
```bash
make ARCH=minirv-npc sim-iverilog IMG=/path/to/image.bin
```

This flow uses the system `iverilog` and `vvp` commands and does not use the OSS
CAD Suite copy of Icarus. Set
`IVERILOG_WAVE=/path/to/wave.vcd` to record a waveform, or run the AM CPU tests
with `VSIM_iverilog=1` to select this simulator.

After ECC synthesis, run the CPU-only functional netlist with either simulator:
```bash
make ARCH=minirv-npc netlist-sim-verilator IMG=/path/to/image.bin
make ARCH=minirv-npc netlist-sim-iverilog IMG=/path/to/image.bin
```

To synthesize both CPU-only ISA variants with ECC and ICsprout55 at 200 MHz:
```bash
make ecc-all
```

The first run downloads ECC, OSS CAD Suite, and the required ICsprout55 Liberty
files into `build/tools`; it does not install them globally. Use `ecc-minirv` or
`ecc-riscv32e` for one variant. `ECC_FREQ_MHZ` and `ECC_SYNTH_STRATEGY` override
the default frequency and Yosys strategy.

To generate the DE-stage single-file synthesizable core for each ISA variant
(run from `npc/`; the DE user-kit must exist, see `bash init.sh mpsoc-osoc-de`):
```bash
make ARCH=minirv-npc de-export     # MiniRV
make ARCH=riscv32e-npc de-export   # full RV32E
```

The generated file is `npc/build/de/<isa>/ysyx_25100261.sv` (`<isa>` is
`minirv` or `riscv32e`). It is also copied to
`mpsoc-osoc-de/user-cores/<isa>/ysyx_25100261.sv` and registered in
`mpsoc-osoc-de/user-cores/<isa>.list`. The `ARCH` platform suffix does not
matter here; only the ISA part selects the variant (or set `DE_ISA=minirv|riscv32e`).
`DE_RESET_PC` (default `30000000`) overrides the reset PC.

To run the DE-stage ASIC SoC user-kit against the synthesizable core:
```bash
bash init.sh mpsoc-osoc-de
make ARCH=minirv-ysyxsoc de-sim
make ARCH=riscv32e-ysyxsoc de-sim
```

This exports a single-file `ysyx_25100261.sv`, registers it as slot 0 in the
user-kit `cores.list`, and then runs the kit's own `make sim CPU_ID=0`. Use
`de-check` or `de-lint` for the corresponding kit targets.
