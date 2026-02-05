NAME = kasan
KASAN_DIR ?= /home/wuser/gitclones/baremetal_kasan

SRC_FILENAMES = kasan.c heap.c rt_utils.c

SRCS = $(addprefix $(KASAN_DIR)/,$(SRC_FILENAMES))
SRCS += $(shell find . -name '*.c')
CFLAGS += -I$(KASAN_DIR) -I$(abspath .)

IS_BUILDKASAN = 1

$(shell mkdir -p ./build)
$(shell cp $(KASAN_DIR)/common.h ./build/mem.h)
# $(shell echo "test")

include $(AM_HOME)/Makefile
