NAME = kasan
KASAN_DIR ?= /home/wuser/gitclones/baremetal_kasan

SRC_FILENAMES = kasan.c heap.c rt_utils.c


ifeq ($(CONFIG_KASAN),y)
SRCS = $(addprefix $(KASAN_DIR)/,$(SRC_FILENAMES))
CFLAGS += -I$(KASAN_DIR) -I$(abspath .)

$(shell mkdir -p ./build)
$(shell cp $(KASAN_DIR)/common.h ./build/mem.h)
else
SRCS = 
endif

SRCS += $(shell find . -name '*.c')

IS_BUILDKASAN = 1

# $(shell echo "test")

include $(AM_HOME)/Makefile
