NETLIST_BUILD_DIR := $(BUILD_DIR)/netlist/$(ARCH)
NETLIST_PROJECT_DIR := $(ECC_PROJECT_ROOT)/$(ARCH_ISA)
NETLIST_QOR := $(NETLIST_PROJECT_DIR)/$(ECC_QOR_REL)
NETLIST_SYNTH_GZ := $(NETLIST_PROJECT_DIR)/runs/default/Synthesis_yosys/output/ysyx_25100261_$(ARCH_ISA)_Synthesis.v.gz
NETLIST_GZ := $(NETLIST_PROJECT_DIR)/runs/default/Synthesis_yosys/output/ysyx_25100261_$(ARCH_ISA)_Synthesis_sim.v.gz
NETLIST_SYNTH_COPY := $(NETLIST_BUILD_DIR)/ysyx_25100261_$(ARCH_ISA)_Synthesis.v.gz
NETLIST_V := $(NETLIST_BUILD_DIR)/ysyx_25100261_$(ARCH_ISA)_Synthesis_sim.v
NETLIST_TOP := ./netlist/netlist_top.sv
NETLIST_MAIN := ./netlist/netlist_main.cpp
NETLIST_CELL_MODELS := \
	$(ICSPROUT55_DIR)/$(ICSPROUT55_LIB_ROOT)/ics55_LLSC_H7CH/verilog/ics55_LLSC_H7CH.v \
	$(ICSPROUT55_DIR)/$(ICSPROUT55_LIB_ROOT)/ics55_LLSC_H7CL/verilog/ics55_LLSC_H7CL.v \
	$(ICSPROUT55_DIR)/$(ICSPROUT55_LIB_ROOT)/ics55_LLSC_H7CR/verilog/ics55_LLSC_H7CR.v
NETLIST_IVERILOG_SIM := $(NETLIST_BUILD_DIR)/iverilog-simv
NETLIST_VERILATOR_DIR := $(NETLIST_BUILD_DIR)/verilator
NETLIST_VERILATOR_SIM := $(NETLIST_VERILATOR_DIR)/netlist-sim
NETLIST_MAX_CYCLES ?= 1000000000
NETLIST_WAVE ?=

$(NETLIST_GZ): $(CHISEL_SRCS) scripts/ecc.mk
	$(MAKE) $(NETLIST_QOR)
	@test -s $@

$(NETLIST_SYNTH_GZ): $(CHISEL_SRCS) scripts/ecc.mk
	$(MAKE) $(NETLIST_QOR)
	@test -s $@

$(NETLIST_SYNTH_COPY): $(NETLIST_SYNTH_GZ)
	@mkdir -p $(@D)
	cp $< $@

netlist: $(NETLIST_SYNTH_COPY)

$(NETLIST_V): $(NETLIST_GZ)
	@mkdir -p $(@D)
	gzip -dc $< > $@.tmp
	mv $@.tmp $@

$(NETLIST_IVERILOG_SIM): $(NETLIST_V) $(NETLIST_TOP) $(NETLIST_CELL_MODELS) scripts/netlist-sim.mk
	$(IVERILOG) -g2012 -Dfunctional -s iverilog_netlist_top -o $@ \
		$(NETLIST_TOP) $(NETLIST_V) $(NETLIST_CELL_MODELS)

netlist-build-iverilog: $(NETLIST_IVERILOG_SIM) $(IVERILOG_PLUGIN)

netlist-sim-iverilog: netlist-build-iverilog
	@test "$(ARCH_PLATFORM)" = npc || { echo "netlist simulation only supports *-npc"; exit 2; }
	@test -n "$(IMG)" || { echo "IMG=<AM binary> is required"; exit 2; }
	$(VVP) -M$(IVERILOG_BUILD_DIR) -mpmem_vpi $(NETLIST_IVERILOG_SIM) \
		+image=$(abspath $(IMG)) +max-cycles=$(NETLIST_MAX_CYCLES) \
		$(if $(NETLIST_WAVE),+wave=$(abspath $(NETLIST_WAVE)))

sim-iverilog-netlist: netlist-sim-iverilog

$(NETLIST_VERILATOR_SIM): $(NETLIST_V) $(NETLIST_TOP) $(NETLIST_MAIN) $(NETLIST_CELL_MODELS) scripts/netlist-sim.mk
	@mkdir -p $(NETLIST_VERILATOR_DIR)
	$(VERILATOR) --cc --exe --build -Wall --timescale "1ns/1ns" --no-timing \
		-D__VERILATOR__ -Dfunctional -Wno-DECLFILENAME -Wno-BLKSEQ \
		-Wno-IMPLICIT -Wno-UNDRIVEN -Wno-UNUSEDSIGNAL -Wno-UNUSEDPARAM \
		-Wno-PINMISSING --top-module NetlistTop \
		--Mdir $(NETLIST_VERILATOR_DIR) -o netlist-sim \
		-CFLAGS "-std=c++20" $(NETLIST_TOP) $(NETLIST_V) $(NETLIST_CELL_MODELS) $(NETLIST_MAIN)

netlist-build-verilator: $(NETLIST_VERILATOR_SIM)

netlist-sim-verilator: netlist-build-verilator
	@test "$(ARCH_PLATFORM)" = npc || { echo "netlist simulation only supports *-npc"; exit 2; }
	@test -n "$(IMG)" || { echo "IMG=<AM binary> is required"; exit 2; }
	$(NETLIST_VERILATOR_SIM) +image=$(abspath $(IMG)) +max-cycles=$(NETLIST_MAX_CYCLES)

.PHONY: netlist netlist-build-iverilog netlist-sim-iverilog sim-iverilog-netlist \
	netlist-build-verilator netlist-sim-verilator
