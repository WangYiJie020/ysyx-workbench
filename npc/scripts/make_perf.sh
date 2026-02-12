#!/bin/bash
export ARCH=riscv32e-ysyxsoc
make clean
make verilog
make -C ../am-kernels/benchmarks/microbench run VSIM_difftest=0 mainargs=train SAN_FLAGS=
