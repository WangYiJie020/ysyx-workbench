BUILD_DIR = ./build
# case sensitive
CPU_DESIGN_NAME = ysyx_25100261

SIM_CSRC_DIR = ./sim
VSIM_OPT?= -O3
SIM_BIN_DIR = $(BUILD_DIR)/bin

COMMIT_HASH = $(shell git rev-parse HEAD)
CXXFLAGS += -DGIT_COMMIT_HASH=$(COMMIT_HASH)

ARCH ?= riscv32e-npc

ARCH_PLATFORM = $(lastword $(subst -, ,$(ARCH)))
ARCH_ISA = $(firstword $(subst -, ,$(ARCH)))

SUPPORTED_ARCHS = minirv-npc minirv-ysyxsoc riscv32e-npc riscv32e-ysyxsoc
ifeq ($(filter $(ARCH),$(SUPPORTED_ARCHS)),)
$(error Unsupported ARCH=$(ARCH), expected one of: $(SUPPORTED_ARCHS))
endif

CXXFLAGS += -DSIM_ARCH_$(shell echo $(ARCH_PLATFORM) | tr a-z A-Z)

SIM_VSRCS =

VERILATOR_FLAGS ?=
VERILATOR_INCDIRS =

ifeq ($(ARCH_PLATFORM),ysyxsoc)
TOP_NAME = SimTop
else
TOP_NAME = TestSoC
endif

SIM_BIN = $(SIM_BIN_DIR)/$(TOP_NAME)-$(ARCH)$(VSIM_OPT)
default: $(SIM_BIN)

CHISEL2V_EMIT_ROOT = $(BUILD_DIR)/gened/verilog

ifeq ($(ARCH_PLATFORM),ysyxsoc)
include ./scripts/ysyxsoc.mk
else
include ./scripts/mytestsoc.mk
endif

VERILATOR = verilator

IMG ?= 

SAN_FLAGS?= #-fsanitize=address

# Performance counter need VPI to fetch handshake signals
ENABLE_VPI ?= 0

ifeq ($(ENABLE_VPI), 1)
VERILATOR_FLAGS += --vpi
endif

# Dependencies for sim
include ./scripts/deplibs.mk
include ./scripts/chisel.mk

VERILATOR_FLAGS += --trace-fst --trace-structs

V2CXX_EMIT_DIR = $(BUILD_DIR)/gened/cxx/$(ARCH)

CORE_VSRCS = $(CHISEL_EMITED_VSRCS)
SYNTH_VSRCS = $(CHISEL_EMITED_VSRCS_SYNTH)

MY_SIM_VSRCS += $(CORE_VSRCS)
MY_SIM_VSRCS += $(CHISEL_EMITED_VSRCS_LAYER)


SIM_VSRCS += $(MY_SIM_VSRCS)
SIM_CSRCS += $(shell find $(abspath $(SIM_CSRC_DIR)) -name "*.c" -or -name "*.cc" -or -name "*.cpp")

SIM_HEADERS = $(shell find $(abspath $(SIM_CSRC_DIR)) -name "*.h" -or -name "*.hpp")

STA_OUT_DIR = $(abspath $(BUILD_DIR)/sta)

# dirs
$(shell mkdir -p $(BUILD_DIR))
$(shell mkdir -p $(SIM_BIN_DIR))
$(shell mkdir -p $(V2CXX_EMIT_DIR))

# rules for verilator

VERILATOR_FLAGS += -MMD -cc \
										--x-assign fast --x-initial fast \
										--autoflush \
										--compiler clang \
										$(addprefix -I,$(VERILATOR_INCDIRS)) \

VERILATOR_FLAGS += --hierarchical
										# --output-split 20000 

COMMON_VERILOG_DEFINES = ysyx_25100261_InlinePrintfLayer
ifdef GITHUB_ACTIONS
COMMON_VERILOG_DEFINES += IS_CI_ENV
endif

# need reserved since both verilator and iverilog will use it
COMMON_VERILOG_DEFINES_FLAGS = $(addprefix -D,$(COMMON_VERILOG_DEFINES))

VERILATOR_FLAGS += $(COMMON_VERILOG_DEFINES_FLAGS)


VERILATOR_CONTROL_FILE = verilator.vlt

# --noassert -O3

INCFLAGS = $(addprefix -I, $(INC_PATH))
CXXFLAGS += $(INCFLAGS) -DTOP_NAME=V$(TOP_NAME) -DENABLE_VPI=$(ENABLE_VPI)
# Verilator's generated makefile contains a few GCC-only warnings (for
# example -Wlogical-op).  Clang treats unknown warnings and several legacy
# diagnostics as errors, so explicitly relax them for the simulator build.
CXXFLAGS += $(SAN_FLAGS) -std=c++20 -g \
	-Wno-unknown-warning-option -Wno-error \
	-Wno-delete-non-abstract-non-virtual-dtor -Wno-reorder-ctor \
	-Wno-unused-private-field # -fno-inline

# when use perf tool, frame pointer is needed
# CXXFLAGS += -fno-omit-frame-pointer

# LDFLAGS += -fuse-ld=mold
# Some bundled static libraries are built without PIE relocation support.
# Disable the toolchain's distro-default PIE mode for the simulator binary.
LDFLAGS += -no-pie

MOLD_EXISTS := $(shell command -v mold 2> /dev/null)
ifneq ($(MOLD_EXISTS),)
# make link faster
  LDFLAGS += -fuse-ld=mold
endif

$(SIM_BIN): $(SIM_DEP_LIBS_BUILD_DONE) $(CHISEL2V_DONE) $(SIM_VSRCS) $(SIM_CSRCS) $(ARCHIVES) $(SIM_HEADERS) $(VERILATOR_CONTROL_FILE)
ifndef IN_LOCK
	@+flock $(SIM_BIN).lock $(MAKE) $@ IN_LOCK=1
else
	@rm -rf $(V2CXX_EMIT_DIR)
	@echo "=== Gen c++ sources from verilog with verilator ==="
	@$(VERILATOR) $(VERILATOR_FLAGS) \
		$(SIM_CSRCS) $(NVBOARD_ARCHIVE) \
		--top-module $(TOP_NAME) $(VERILATOR_CONTROL_FILE) $(SIM_VSRCS) \
		$(addprefix -CFLAGS , $(CXXFLAGS)) \
		$(addprefix -LDFLAGS , $(LDFLAGS)) \
		--Mdir $(V2CXX_EMIT_DIR) --exe -o $(abspath $(SIM_BIN))
	@echo "=== Build sim binary with clang++ ==="
	@echo "clang++ version: $(shell clang++ -dumpversion)"
	@$(MAKE) -s -C $(V2CXX_EMIT_DIR) -f V$(TOP_NAME).mk -j$(shell nproc) OPT_FAST=$(VSIM_OPT) CXX=clang++ LINK=clang++ AR="ar -T"
endif

verilog-lint: verilog
	$(info Lint verilog files with verilator)
	@$(VERILATOR) --lint-only $(VERILATOR_FLAGS) -Wall $(SYNTH_VSRCS)

all: default

test:
	$(MILL) -i $(CHISEL_DESIGN).test

SYNTH_VSRCS_ALLINONE = $(BUILD_DIR)/$(CPU_DESIGN_NAME).v
$(SYNTH_VSRCS_ALLINONE): $(CHISEL2V_DONE)
	@echo "# Merge verilog files into one file $(SYNTH_VSRCS_ALLINONE)"
	@cat $(SYNTH_VSRCS) > $(SYNTH_VSRCS_ALLINONE)

verilog: $(SYNTH_VSRCS_ALLINONE)

help:
	$(MILL) -i $(CHISEL_DESIGN).runMain Elaborate --help

reformat:
	$(MILL) -i __.reformat

checkformat:
	$(MILL) -i __.checkFormat

bsp:
	$(MILL) -i mill.bsp.BSP/install

idea:
	$(MILL) -i mill.idea.GenIdea/idea

clean:
	-rm -rf $(BUILD_DIR)

clean-bin:
	-rm -rf $(SIM_BIN_DIR)

.PHONY: test help reformat checkformat clean

ARGS?=
sim: $(SIM_BIN) 
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@echo "=== ENV ==="
	@echo "VSIM_OPT=$(VSIM_OPT)"
	@echo "ENABLE_VPI=$(ENABLE_VPI)"
	@echo "=== SIM BEGIN ==="
	@LSAN_OPTIONS=suppressions=asan_suppress.txt $^ $(ARGS) $(IMG)

include ./scripts/ecc.mk

-include ../Makefile
