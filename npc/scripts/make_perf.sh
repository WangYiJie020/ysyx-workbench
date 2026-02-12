#!/bin/bash
make clean
make -C ../am-kernels/benchmarks/microbench ARCH=riscv32e-ysyxsoc run VSIM_difftest=0 mainargs=train SAN_FLAGS=
