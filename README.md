# "一生一芯"工程项目

这是"一生一芯"的工程项目. 通过运行
```bash
bash init.sh subproject-name
```
进行初始化, 具体请参考[实验讲义][lecture note].

[lecture note]: https://ysyx.oscc.cc/docs/

## 生成 DE 阶段的 .sv 文件

DE 阶段需要单文件可综合的 CPU 核. 先初始化 DE user-kit, 再在 `npc/` 下导出
对应 ISA 变体 (MiniRV / 完整 RV32E):
```bash
bash init.sh mpsoc-osoc-de
make -C npc ARCH=minirv-npc de-export     # MiniRV
make -C npc ARCH=riscv32e-npc de-export   # RV32E
```

生成的 `ysyx_25100261.sv` 位于 `npc/build/de/<isa>/`, 同时会复制到
`mpsoc-osoc-de/user-cores/<isa>/` 并登记到 `<isa>.list` (`<isa>` 为 `minirv` 或
`riscv32e`). 将 `de-export` 换成 `de-check` / `de-lint` / `de-sim` 可运行 user-kit
对应的检查、lint 与仿真, 详见 [npc/README.md](npc/README.md).
