INC_PATH ?=

all:
	@echo "Write this Makefile by your self."

CSRC_DIR := ./csrc
VSRC_DIR := ./vsrc

CPP_SRCS := $(shell find $(CSRC_DIR) -type f -name "*.cpp")
V_SRCS   := $(shell find $(VSRC_DIR) -type f -name "*.v")

# rules for NVBoard
include $(NVBOARD_HOME)/scripts/nvboard.mk

# rules for verilator
INCFLAGS = $(addprefix -I, $(INC_PATH))
CXXFLAGS += $(INCFLAGS)

sim:
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@echo "Write this Makefile by your self."

	verilator --cc --exe --build --trace -j 0 -Wall $(addprefix -CFLAGS , $(CXXFLAGS)) $(CPP_SRCS) $(V_SRCS)
	./obj_dir/Vtop

include ../Makefile
