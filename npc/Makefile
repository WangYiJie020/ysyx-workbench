INC_PATH ?=

NXDC_FILES = constr/top.nxdc

CSRC_DIR := ./csrc
VSRC_DIR := ./vsrc

CPP_SRCS := $(shell find $(CSRC_DIR) -type f -name "*.cpp")
V_SRCS   := $(shell find $(VSRC_DIR) -type f -name "*.v")

BUILD_DIR = ./build

$(shell mkdir -p $(BUILD_DIR))

# constraint file
SRC_AUTO_BIND = $(abspath $(BUILD_DIR)/auto_bind.cpp)
$(SRC_AUTO_BIND): $(NXDC_FILES)
	python3 $(NVBOARD_HOME)/scripts/auto_pin_bind.py $^ $@

CPP_SRCS += $(SRC_AUTO_BIND)

# rules for NVBoard
include $(NVBOARD_HOME)/scripts/nvboard.mk

# rules for verilator
INCFLAGS = $(addprefix -I, $(INC_PATH))
CXXFLAGS += $(INCFLAGS)

bind:

all:
	@echo "Write this Makefile by your self."
sim:
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@echo "Write this Makefile by your self."

	verilator --cc --exe --build --trace -j 0 -Wall
	 $(addprefix -CFLAGS , $(CXXFLAGS))\
	  	$(CPP_SRCS) $(V_SRCS)\
	 	$(addprefix -LDFLAGS , $(LDFLAGS))
	./obj_dir/Vtop

include ../Makefile
