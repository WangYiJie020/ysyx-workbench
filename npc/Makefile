BUILD_DIR = ./build
TOP_NAME = Top # case sensitive

SIM_CSRC_DIR = ./csrc

SIM_IMG ?=

# Chisel
CHISEL_DESIGN = design
CHISEL2V_EMIT_DIR = $(BUILD_DIR)/chisel2v


# Verilator
VERILATOR = verilator
VERILATOR_CFLAGS += -MMD --build -cc -O3 \
										--x-assign fast --x-initial fast --noassert

V2CXX_EMIT_DIR = $(BUILD_DIR)/v2cxx
SIM_BIN = $(BUILD_DIR)/$(TOP_NAME)

INC_PATH += $(abspath $(V2CXX_EMIT_DIR))

SIM_VSRCS = $(shell find $(abspath $(CHISEL2V_EMIT_DIR)) -name "*.v" -or -name "*.sv")
SIM_CSRCS = $(shell find $(abspath $(SIM_CSRC_DIR)) -name "*.c" -or -name "*.cc" -or -name "*.cpp")

# sdb
INC_PATH += $(abspath ../sdb/include)
LDFLAGS += $(SAN_FLAGS) -L$(abspath ../sdb/build) -lsdb
ARCHIVES += $(NVBOARD_ARCHIVE) $(abspath ../sdb/build/libsdb.a)

# dirs
$(shell mkdir -p $(BUILD_DIR))
$(shell mkdir -p $(V2CXX_EMIT_DIR))
$(shell mkdir -p $(CHISEL2V_EMIT_DIR))

default: $(SIM_BIN)

# NVBOARD
NXDC_FILES = $(shell find $(abspath .) -name "*.nxdc")
NVB_AUTOBIND_SRC = $(abspath $(BUILD_DIR)/auto_bind.cpp)
$(NVB_AUTOBIND_SRC): $(NXDC_FILES)
	python3 $(NVBOARD_HOME)/scripts/auto_pin_bind.py $^ $@

SIM_CSRCS += $(NVB_AUTOBIND_SRC)

# rules for NVBoard
include $(NVBOARD_HOME)/scripts/nvboard.mk

# rules for verilator
INCFLAGS = $(addprefix -I, $(INC_PATH))
CXXFLAGS += $(INCFLAGS) -DTOP_NAME="\"V$(TOP_NAME)\""
SAN_FLAGS?= #SAN_FLAGS+=-fsanitize=address
CXXFLAGS += $(SAN_FLAGS) -std=c++20

$(SIM_BIN): $(SIM_VSRCS) $(SIM_CSRCS) $(ARCHIVES) verilog
	@rm -rf $(V2CXX_EMIT_DIR)
	$(VERILATOR) $(VERILATOR_CFLAGS) \
		--top-module $(TOP_NAME) $(SIM_CSRCS) $(SIM_VSRCS) $(NVBOARD_ARCHIVE) \
		$(addprefix -CFLAGS , $(CXXFLAGS)) $(addprefix -LDFLAGS , $(LDFLAGS)) \
		--Mdir $(V2CXX_EMIT_DIR) --exe -o $(abspath $(SIM_BIN))

all: default

test:
	mill -i $(CHISEL_DESIGN).test

verilog:
	$(call git_commit, "generate verilog")
	mkdir -p $(BUILD_DIR)
	mill -i $(CHISEL_DESIGN).runMain Elaborate --target-dir $(CHISEL2V_EMIT_DIR)

help:
	mill -i $(CHISEL_DESIGN).runMain Elaborate --help

reformat:
	mill -i __.reformat

checkformat:
	mill -i __.checkFormat

bsp:
	mill -i mill.bsp.BSP/install

idea:
	mill -i mill.idea.GenIdea/idea

clean:
	-rm -rf $(BUILD_DIR)

.PHONY: test verilog help reformat checkformat clean

ARGS?=
sim: $(SIM_BIN)
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@echo "Run the simulation"
	@$^ $(ARGS) $(SIM_IMG)

-include ../Makefile
