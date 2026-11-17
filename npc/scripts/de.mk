# Thin wrapper for the DE-stage ASIC SoC user-kit.
# Export a synthesizable single-file core, register it in cores.list, then
# call the kit's own make check/lint/sim. This is not wired into npc sim.

DE_HOME ?= $(abspath ../mpsoc-osoc-de)
DE_ISA ?= $(ARCH_ISA)
DE_CPU_ID ?= 0
DE_CORE_MODULE ?= $(CPU_DESIGN_NAME)
DE_RESET_PC ?= 30000000

DE_EXPORT_DIR := $(abspath $(BUILD_DIR)/de/$(DE_ISA))
DE_EMIT_DIR := $(DE_EXPORT_DIR)/$(DE_ISA)
DE_CORE_SV := $(DE_EXPORT_DIR)/$(DE_CORE_MODULE).sv
DE_USER_CORE_DIR := $(DE_HOME)/user-cores/$(DE_ISA)
DE_USER_CORE_SV := $(DE_USER_CORE_DIR)/$(DE_CORE_MODULE).sv
DE_MANIFEST := $(DE_HOME)/user-cores/$(DE_ISA).list

ifeq ($(filter $(DE_ISA),minirv riscv32e),)
$(error DE_ISA=$(DE_ISA) is not supported, expected minirv or riscv32e)
endif

$(DE_CORE_SV): $(CHISEL_SRCS)
	@test -d "$(DE_HOME)" || { \
		echo "DE user-kit missing: $(DE_HOME)"; \
		echo "Clone it with: bash init.sh mpsoc-osoc-de"; \
		exit 2; \
	}
	@rm -rf $(DE_EXPORT_DIR)
	$(MILL) -i $(CHISEL_DESIGN).runMain Elaborate \
		--target-dir $(DE_EXPORT_DIR) \
		--isa $(DE_ISA) --platform de --reset-pc 0x$(DE_RESET_PC)
	@mkdir -p $(dir $@)
	@files=$$(grep -vE '$(CHISEL_UNSYNTH_KEYWORDS)' $(DE_EMIT_DIR)/filelist.f | awk 'NF { print "$(DE_EMIT_DIR)/" $$0 }'); \
		test -n "$$files" || { echo "no synthesizable Verilog in $(DE_EMIT_DIR)"; exit 2; }; \
		cat $$files > $@
	@grep -q '^module $(DE_CORE_MODULE)(' $@ || { \
		echo "exported core is missing module $(DE_CORE_MODULE): $@"; exit 2; \
	}

$(DE_MANIFEST): $(DE_CORE_SV)
	@mkdir -p $(DE_USER_CORE_DIR)
	@cp -f $(DE_CORE_SV) $(DE_USER_CORE_SV)
	@printf '%s\n' \
		'# Generated for the DE user-kit. Slot 0 is the exported NPC core.' \
		'0 $(DE_CORE_MODULE) $(DE_ISA)/$(DE_CORE_MODULE).sv' \
		> $@
	@echo "registered $(DE_CORE_MODULE) from $(DE_USER_CORE_SV)"

de-export: $(DE_MANIFEST)
	@echo $(DE_USER_CORE_SV)
	@echo $(DE_MANIFEST)

de-check: $(DE_MANIFEST)
	$(MAKE) -C $(DE_HOME) check CPU_ID=$(DE_CPU_ID) USER_CORES=$(abspath $(DE_MANIFEST))

de-lint: $(DE_MANIFEST)
	$(MAKE) -C $(DE_HOME) lint CPU_ID=$(DE_CPU_ID) USER_CORES=$(abspath $(DE_MANIFEST))

de-sim: $(DE_MANIFEST)
	CCACHE_DISABLE=1 $(MAKE) -C $(DE_HOME) sim CPU_ID=$(DE_CPU_ID) \
		USER_CORES=$(abspath $(DE_MANIFEST))

de-trace: $(DE_MANIFEST)
	CCACHE_DISABLE=1 $(MAKE) -C $(DE_HOME) trace CPU_ID=$(DE_CPU_ID) \
		USER_CORES=$(abspath $(DE_MANIFEST))

.PHONY: de-export de-check de-lint de-sim de-trace
