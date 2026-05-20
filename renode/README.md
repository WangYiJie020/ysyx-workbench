# ysyxSoC Renode GDB Debugging

This directory contains a minimal Renode setup for RV32E bare-metal images built
for the ysyxSoC memory map.

## Start Renode GDB Server

Pass the ELF explicitly with `server` mode:

```sh
./run-gdb.sh server path/to/program.elf
```

The script starts Renode with:

- reset PC at `0x30000000`
- UART at `0x10000000`
- CLINT at `0x02000000`
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
./run-gdb.sh gdb path/to/program.elf
```

`run-gdb.sh` loads startup commands from `start.gdb`, which connects to
`:3333` by default. If `GDB_PORT` was changed for the server, update
`start.gdb` to use the same port before connecting.

Renode loads the raw flash image at `0x30000000`. The image is generated from
the ELF's physical addresses (LMA). The runtime sections are not preloaded into
their VMA locations; the in-image bootloader performs those copies.

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
