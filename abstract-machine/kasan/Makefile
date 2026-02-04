NAME = kasan
KASAN_DIR ?= /home/wuser/gitclones/baremetal_kasan

SRC_FILENAMES = kasan.c heap.c rt_utils.c

SRCS = $(addprefix $(KASAN_DIR)/,$(SRC_FILENAMES))
SRCS += $(KASAN_DIR)/third_party/printf/printf.c
CFLAGS += -I$(KASAN_DIR) -I$(KASAN_DIR)/third_party/printf

IS_BUILDKASAN = 1

include $(AM_HOME)/Makefile
