AM_SRCS := riscv/ysyxsoc/start.S \
						 riscv/ysyxsoc/trm.c \
           riscv/ysyxsoc/ioe.c \
           riscv/ysyxsoc/gpu.c \
           riscv/ysyxsoc/input.c \
           riscv/ysyxsoc/timer.c \
           riscv/npc/cte.c \
           riscv/npc/trap.S \
           platform/dummy/vme.c \
           platform/dummy/mpe.c

ifeq ($(VSIM_zero_uninit_ram), 1)
CFLAGS += -DSKIP_BSS_CLEAR
endif

CFLAGS += -g

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
	@cd $(AM_HOME)/../ysyxSoC/ready-to-run/minirv && bash gen.sh $(IMAGE).elf $(IMAGE).bin

run: insert-arg
	@$(MAKE) -C $(NPC_HOME) sim ARCH=$(ARCH) IMG=$(IMAGE).bin ARGS=-b
