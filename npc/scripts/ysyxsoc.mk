# ysyxSoC

ifneq ($(ARCH_PLATFORM),ysyxsoc)
$(error Shouldn't include soc.mk when not building ysyxSoC, something is wrong)
endif

CXXFLAGS += -DSIM_SOC

SOC_HOME ?= ../ysyxSoC
SOC_PERIP_VSRCS = $(shell find $(abspath $(SOC_HOME)/perip) -name "*.v")
SOC_PERIP_VSRCS += $(shell find $(abspath $(SOC_HOME)/perip) -name "*.sv")
SOC_GENED_FILE = $(abspath $(SOC_HOME)/ready-to-run/minirv/ElaborateTop.v)

$(SOC_GENED_FILE):
	@test -f $@ || $(MAKE) -C $(SOC_HOME) verilog

SIM_VSRCS += $(SOC_GENED_FILE)
SIM_VSRCS += $(SOC_PERIP_VSRCS)

VERILATOR_INCDIRS += $(abspath $(SOC_HOME)/perip/uart16550/rtl)
VERILATOR_INCDIRS += $(abspath $(SOC_HOME)/perip/spi/rtl)

RESET_PC = 30000000

VERILATOR_FLAGS += --timescale "1ns/1ns" --no-timing
VERILATOR_FLAGS += -D__VERILOG__ -DPDK_BEHAV -D__UART_TO_CONSOLE__
VERILATOR_FLAGS += -Wno-SYMRSVDWORD # make uart verilog happy

VERILATOR_FLAGS += -Wno-WIDTHEXPAND # make chiplink happy
# VERILATOR_FLAGS += -Wno-UNOPTFLAT

ENABLE_NVBOARD ?= 1
CXXFLAGS += -DENABLE_NVBOARD=$(ENABLE_NVBOARD)

ifeq ($(ENABLE_NVBOARD),1)
NVBOARD_HOME ?= $(abspath ../nvboard)
export NVBOARD_HOME
NVBOARD_NXDC = $(abspath ./nvboard.nxdc)
NVBOARD_AUTO_BIND = $(abspath $(BUILD_DIR)/nvboard/auto_bind.cpp)

$(NVBOARD_AUTO_BIND): $(NVBOARD_NXDC)
	@mkdir -p $(dir $@)
	@NVBOARD_HOME=$(NVBOARD_HOME) python3 \
		$(NVBOARD_HOME)/scripts/auto_pin_bind.py $< $@

SIM_CSRCS += $(NVBOARD_AUTO_BIND)
include $(NVBOARD_HOME)/scripts/nvboard.mk
endif
