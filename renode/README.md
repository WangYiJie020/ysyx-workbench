# RV32E Renode GDB Debugging

This directory contains a minimal Renode setup for RV32E bare-metal images built
for the `riscv32e-ysyxsoc` and `riscv32e-npc` memory maps.

## Start Renode GDB Server

Pass the ELF explicitly with `server` mode:

```sh
ARCH=riscv32e-ysyxsoc ./run-gdb.sh server path/to/program.elf
ARCH=riscv32e-npc ./run-gdb.sh server ./dut/rtthread-riscv32e-npc.elf
```

The script selects the matching `$ARCH.resc` and starts Renode with the matching
memory map.

- `riscv32e-ysyxsoc`: reset PC at `0x30000000`, raw image loaded at flash `0x30000000`
- `riscv32e-npc`: reset PC at `0x80000000`, raw image loaded at PMEM `0x80000000`
- UART at `0x10000000`
- CLINT timer reads at `0x02000048` and `0x0200004c`
- GDB server on port `3333`

The wrapper expects a raw flash image next to the ELF with the same basename
and a `.bin` suffix, then Renode loads that image with `LoadBinary`. This avoids
Renode's slower ELF loader while keeping the original ELF available to GDB for
symbols.

Override the GDB server port with `GDB_PORT=<port>` if needed. When using a
non-default port, update `start.gdb` to connect to the same port.

## Connect GDB

Use the matching wrapper:

```sh
ARCH=riscv32e-npc ./run-gdb.sh gdb ./dut/rtthread-riscv32e-npc.elf
```

`run-gdb.sh` loads startup commands from `start.gdb`, which connects to
`:3333` by default. If `GDB_PORT` was changed for the server, update
`start.gdb` to use the same port before connecting.

For `riscv32e-ysyxsoc`, Renode loads the raw image at `0x30000000`; the
in-image bootloader copies runtime sections to their VMA locations. For
`riscv32e-npc`, there is no bootloader, so Renode loads the raw image directly
at `0x80000000`.

## Restart After a CPU Fault

If the CPU faults and Renode closes the GDB server, keep the Renode monitor open
and run:

```renode
runMacro $restart_gdb
```

Then reconnect GDB with the same command used initially. The macro pauses the
machine, stops any existing GDB server, resets the CPU, sets `cpu PC` to
`0x30000000`, and starts the GDB server again on the configured `GDB_PORT`.

This fast restart does not reload the ELF. Use it when the program image in
flash is still intact and the runtime can reinitialize RAM on boot.

If memory contents must be restored from the flash image too, use the full
restart:

```renode
runMacro $reload_gdb
```
