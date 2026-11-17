# Repository Guidelines

## Project Structure & Module Organization

This workbench combines several related hardware and systems projects. `npc/` contains the Chisel CPU design, Verilator/iverilog simulation code, and hardware tests. `nemu/` is the reference emulator; its implementation is under `nemu/src/` and configuration lives in `nemu/configs/` and `nemu/Kconfig`. `abstract-machine/` provides architecture-neutral runtime libraries (`am/`, `klib/`, and `kasan/`). Shared debugger and trace support is in `sdb/`; `cachesim/` and `branchsim/` contain standalone C++ simulators. Integration changes for external projects belong in `patch/`. GitHub Actions definitions are in `.github/`.

## Build, Test, and Development Commands

Run builds from the relevant subproject; the root `make` only directs contributors elsewhere. Set `NEMU_HOME`, `AM_HOME`, and `NPC_HOME` to their repository directories (or run `bash init.sh npc` and initialize other required subprojects).

- `make -C npc test`: run the Chisel test suite through Mill.
- `make -C npc verilog`: elaborate the design into `npc/build/`.
- `make -C npc sim IMG=/path/to/image`: build and run the Verilator simulator.
- `make -C npc verilog-lint`: lint generated RTL with Verilator.
- `make -C nemu menuconfig && make -C nemu`: configure and build NEMU.
- `make -j16 ARCH=riscv32e-npc -C am-kernels/tests/cpu-tests run`: run full RV32E core CPU tests when `am-kernels/` is initialized.
- `make -j16 ARCH=minirv-npc -C am-kernels/tests/cpu-tests run`: run the AM-rewritten eight-instruction MiniRV tests.
- `make -C npc ecc-all`: synthesize both CPU-only variants with ECC and ICsprout55 at the default 200 MHz target.
- `make -C npc clean`: remove generated NPC artifacts.

## Current CPU Architecture & Area Constraints

NPC is an area-oriented, non-pipelined multi-cycle RV32E core. Keep instruction fetch and LSU operations on their existing SimpleBus request/response handshakes; do not reintroduce caches, branch prediction, BTBs, pipeline registers, performance counters, or AXI arbitration. Architectural register and CSR updates, difftest DPI calls, and PC retirement happen only in the commit state. Preserve `ecall`, `mret`, CSR instructions, `mstatus`, `mtvec`, `mepc`, `mcause`, 64-bit `mcycle`, `mvendorid`, and the student-number `marchid`.

ECC evaluates the CPU core only, excluding ysyxSoC. The ICsprout55 target is 200 MHz with hard post-synthesis cell-area limits of less than 10000 um2 for MiniRV and less than 11000 um2 for full RV32E. The previous area-minimized baseline was 7713 um2 / 305 MHz for MiniRV and 8902 um2 / 215 MHz for RV32E. Treat only regressions beyond the current limits as area failures.

The current implementation deliberately uses a conventional two-read-port, unreset DFF register file and completes 32-bit addition/subtraction in one execute cycle. The iterative shifter still exchanges latency for area, while CSR storage retains its full architectural width. Scala elaboration removes full RV32E shift, compare, logic, halfword-access, and immediate-selection paths from the eight-instruction MiniRV build, and MiniRV byte accesses use fixed four-way selection instead of generic dynamic shifters. Do not restore the rotating single-read-port register file, serial bit-at-a-time adder, or compact CSR representation solely to recover the old area baseline. Keep CSR old/new values in existing operand/result storage before commit.

## Coding Style & Naming Conventions

Follow the style already present in the file being changed. C uses GNU11, C++ uses C++17 or C++20 depending on the component, and Scala/Chisel sources use Scala conventions: `UpperCamelCase` modules/types and `lowerCamelCase` values. Preserve Makefile tab indentation. Run `make -C npc reformat` for Scala formatting and `make -C npc checkformat` before submitting. Do not commit generated `build/` trees, waveforms, or local tool outputs.

## Testing Guidelines

Keep tests close to their component (for example, Chisel specs under `npc/design/test/`). Name Scala tests `*Spec.scala`; use existing AM test layouts for C tests. Run the narrowest relevant test first, then the CI-representative command above. Hardware changes should also pass `verilog-lint` and generate the expected `ysyx_25100261.v` or `.sv` top-level output. During area optimization, run the two `*-npc` CPU-test variants first and do not spend time on `*-ysyxsoc`; after both ECC area limits are met, run `riscv32e-ysyxsoc` and `minirv-ysyxsoc` with `-j16`. Run the four suites sequentially so total concurrency stays at 16.

## Commit & Pull Request Guidelines

Recent commits use short, imperative, lowercase subjects such as `fix duplicate oss-cad cache` and `refactor soc memory sim`; optional component prefixes such as `JYD:` are acceptable. Keep each commit focused. Pull requests should explain the behavior changed, list exact test commands and results, link related issues, and include logs or waveform/screenshot evidence when simulation behavior or visible output changes.
