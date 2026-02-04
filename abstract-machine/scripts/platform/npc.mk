AM_SRCS := riscv/npc/start.S \
           riscv/npc/trm.c \
           riscv/npc/ioe.c \
           riscv/npc/timer.c \
           riscv/npc/input.c \
           riscv/npc/cte.c \
           riscv/npc/trap.S \
           platform/dummy/vme.c \
           platform/dummy/mpe.c

CFLAGS += -g

# CFLAGS += -fsanitize=address
CFLAGS += -fsanitize=kernel-address

# KASAN configuration
TARGET_DRAM_START := 0x80000000
TARGET_DRAM_END := 0x8fffffff
UART_BASE_ADDRESS := 0x10000000

KASAN_SHADOW_MAPPING_OFFSET := 0x77000000
KASAN_SHADOW_MEMORY_START := 0x87000000
KASAN_SHADOW_MEMORY_SIZE := 0x2000000

# CFLAGS += -mllvm -asan-mapping-offset=$(KASAN_SHADOW_MAPPING_OFFSET)

CFLAGS += -DKASAN_SHADOW_MAPPING_OFFSET=$(KASAN_SHADOW_MAPPING_OFFSET)
CFLAGS += -DKASAN_SHADOW_MEMORY_START=$(KASAN_SHADOW_MEMORY_START)
CFLAGS += -DKASAN_SHADOW_MEMORY_SIZE=$(KASAN_SHADOW_MEMORY_SIZE)

CFLAGS += -DTARGET_DRAM_START=$(TARGET_DRAM_START)
CFLAGS += -DTARGET_DRAM_END=$(TARGET_DRAM_END)
CFLAGS += -DUART_BASE_ADDRESS=$(UART_BASE_ADDRESS)

CFLAGS += -ffreestanding

# KASan-specific compiler options
KASAN_SANITIZE_STACK := 1
KASAN_SANITIZE_GLOBALS := 1

KASAN_CC_FLAGS := -fsanitize=kernel-address
KASAN_CC_FLAGS += -fno-builtin
KASAN_CC_FLAGS += -mllvm -asan-mapping-offset=$(KASAN_SHADOW_MAPPING_OFFSET)
KASAN_CC_FLAGS += -mllvm -asan-instrumentation-with-call-threshold=0
KASAN_CC_FLAGS += -mllvm -asan-stack=$(KASAN_SANITIZE_STACK)
KASAN_CC_FLAGS += -mllvm -asan-globals=$(KASAN_SANITIZE_GLOBALS)
KASAN_CC_FLAGS += -DKASAN_ENABLED

CFLAGS += $(KASAN_CC_FLAGS)

CFLAGS    += -fdata-sections -ffunction-sections
LDSCRIPTS += $(AM_HOME)/scripts/linker.ld
LDFLAGS   += --defsym=_pmem_start=0x80000000 --defsym=_entry_offset=0x0
LDFLAGS   += --gc-sections -e _start

MAINARGS_MAX_LEN = 64
MAINARGS_PLACEHOLDER = the_insert-arg_rule_in_Makefile_will_insert_mainargs_here
CFLAGS += -DMAINARGS_MAX_LEN=$(MAINARGS_MAX_LEN) -DMAINARGS_PLACEHOLDER=$(MAINARGS_PLACEHOLDER)

insert-arg: image
	@python $(AM_HOME)/tools/insert-arg.py $(IMAGE).bin $(MAINARGS_MAX_LEN) $(MAINARGS_PLACEHOLDER) "$(mainargs)"

image: image-dep
	@$(OBJDUMP) -d $(IMAGE).elf > $(IMAGE).txt
	@echo + OBJCOPY "->" $(IMAGE_REL).bin
	@$(OBJCOPY) -S --set-section-flags .bss=alloc,contents -O binary $(IMAGE).elf $(IMAGE).bin

run: insert-arg
	#echo "TODO: add command here to run simulation"
	@$(MAKE) -C $(NPC_HOME) sim SIM_IMG=$(IMAGE).bin ARGS='-b'

.PHONY: insert-arg
