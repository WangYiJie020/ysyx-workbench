# npctestsoc

TOP_NAME = NPCTestSoC

SOC_HOME = $(BUILD_DIR)/npctestsoc

SOC_LAYER_VSRCS = $(shell find $(abspath $(SOC_HOME)) -name "layers-*")
SOC_VSRCS = $(call rd_filelist_indir, $(SOC_HOME))

SIM_VSRCS += $(SOC_VSRCS)
SIM_VSRCS += $(SOC_LAYER_VSRCS)

VERILATOR_INCDIRS += $(shell find $(abspath $(SOC_HOME)) -type d)
VERILATOR_FLAGS += -D$(CPU_DESIGN_NAME)_RESET_PC=32\'h80000000
