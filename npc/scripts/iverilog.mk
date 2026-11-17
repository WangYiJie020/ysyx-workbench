IVERILOG ?= iverilog
VVP ?= vvp
IVERILOG_VPI_HEADER := $(firstword $(wildcard \
	/usr/include/iverilog/vpi_user.h \
	/usr/share/verilator/include/vltstd/vpi_user.h))
IVERILOG_VPI_INCDIR := $(patsubst %/vpi_user.h,%,$(IVERILOG_VPI_HEADER))
IVERILOG_MAX_CYCLES ?= 1000000000
IVERILOG_WAVE ?=

IVERILOG_BUILD_DIR := $(BUILD_DIR)/iverilog/$(ARCH)
IVERILOG_EMIT_DIR := $(BUILD_DIR)/gened/iverilog/$(ARCH)
IVERILOG_EMIT_DONE := $(IVERILOG_BUILD_DIR)/emit.done
IVERILOG_SIM := $(IVERILOG_BUILD_DIR)/simv
IVERILOG_PLUGIN := $(IVERILOG_BUILD_DIR)/pmem_vpi.vpi
IVERILOG_TOP := ./iverilog/iverilog_top.sv
IVERILOG_VPI_SRC := ./iverilog/pmem_vpi.c
IVERILOG_VSRCS = $(shell find $(IVERILOG_EMIT_DIR)/$(ARCH_ISA) -name '*.sv' ! -path '*/verification/*' | sort)
IVERILOG_INCDIRS = $(shell find $(IVERILOG_EMIT_DIR)/$(ARCH_ISA) -type d | sort)

$(IVERILOG_EMIT_DONE): $(CHISEL_SRCS)
	@mkdir -p $(IVERILOG_BUILD_DIR)
	@rm -rf $(IVERILOG_EMIT_DIR)
	$(MILL) -i $(CHISEL_DESIGN).runMain Elaborate \
		--target-dir $(IVERILOG_EMIT_DIR) \
		--isa $(ARCH_ISA) --platform iverilog --reset-pc 0x80000000
	@touch $@

$(IVERILOG_SIM): $(IVERILOG_EMIT_DONE) $(IVERILOG_TOP)
	$(IVERILOG) -g2012 -s iverilog_top $(addprefix -I,$(IVERILOG_INCDIRS)) -o $@ \
		$(IVERILOG_TOP) $(IVERILOG_VSRCS)

$(IVERILOG_PLUGIN): $(IVERILOG_VPI_SRC)
	@mkdir -p $(IVERILOG_BUILD_DIR)
	@test -n "$(IVERILOG_VPI_HEADER)" || { \
		echo "vpi_user.h not found; install the system iverilog-devel package"; exit 2; \
	}
	$(CC) -std=c11 -fPIC -Wall -Wextra -Wshadow -shared \
		-I$(IVERILOG_VPI_INCDIR) -o $@ $(IVERILOG_VPI_SRC)

iverilog-build: $(IVERILOG_SIM) $(IVERILOG_PLUGIN)

iverilog-sim: iverilog-build
	@test "$(ARCH_PLATFORM)" = npc || { echo "iverilog only supports the CPU-only *-npc targets"; exit 2; }
	@test -n "$(IMG)" || { echo "IMG=<AM binary> is required"; exit 2; }
	$(VVP) -M$(IVERILOG_BUILD_DIR) -mpmem_vpi $(IVERILOG_SIM) \
		+image=$(abspath $(IMG)) +max-cycles=$(IVERILOG_MAX_CYCLES) \
		$(if $(IVERILOG_WAVE),+wave=$(abspath $(IVERILOG_WAVE)))

sim-iverilog: iverilog-sim

.PHONY: iverilog-build iverilog-sim sim-iverilog
