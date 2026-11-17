# Project-local ECC/ICsprout55 synthesis flow.

ECC_VERSION ?= v0.1.0-alpha.10
ECC_FREQ_MHZ ?= 200
ECC_SYNTH_STRATEGY ?= BALANCE 0
ECC_CORE_RESET_PC ?= 30000000
ECC_ISAS := minirv riscv32e

ECC_BUILD_ROOT := $(abspath $(BUILD_DIR)/ecc)
ECC_TOOLS_DIR := $(abspath $(BUILD_DIR)/tools)
ECC_DOWNLOAD_DIR := $(ECC_TOOLS_DIR)/downloads
ECC_DIR := $(ECC_TOOLS_DIR)/ecc
ECC_BIN := $(ECC_DIR)/ecc
ECC_READY := $(ECC_DIR)/.npc-ready
ECC_ARCHIVE := $(ECC_DOWNLOAD_DIR)/ecc-cli-linux-x86_64.tar.gz
ECC_ARCHIVE_SHA256 := fc3daaca24dddb04ba3490329042f52da05190da03c3831042293dd0cbffdca6
ECC_URL := https://github.com/openecos-projects/ecc/releases/download/$(ECC_VERSION)/ecc-cli-linux-x86_64.tar.gz

OSS_CAD_VERSION ?= 2026-08-27
OSS_CAD_DIR := $(ECC_TOOLS_DIR)/oss-cad-suite
OSS_CAD_BIN := $(OSS_CAD_DIR)/bin
OSS_CAD_YOSYS := $(OSS_CAD_BIN)/yosys
OSS_CAD_READY := $(OSS_CAD_DIR)/.npc-ready
OSS_CAD_ARCHIVE := $(ECC_DOWNLOAD_DIR)/oss-cad-suite-linux-x64-20260827.tgz
OSS_CAD_ARCHIVE_SHA256 := c3ffafe1549d5bf321b85e6ac30b3bc6af0af72093d5f45117380bcc0645a8e2
OSS_CAD_URL := https://github.com/YosysHQ/oss-cad-suite-build/releases/download/$(OSS_CAD_VERSION)/oss-cad-suite-linux-x64-20260827.tgz

ICSPROUT55_DIR := $(ECC_TOOLS_DIR)/icsprout55-pdk
ICSPROUT55_READY := $(ICSPROUT55_DIR)/.npc-synthesis-ready
ICSPROUT55_LIB_ROOT := IP/STD_cell/ics55_LLSC_H7C_V1p10C100
ICSPROUT55_LIB_TARGETS := \
	$(ICSPROUT55_LIB_ROOT)/ics55_LLSC_H7CH/liberty \
	$(ICSPROUT55_LIB_ROOT)/ics55_LLSC_H7CL/liberty \
	$(ICSPROUT55_LIB_ROOT)/ics55_LLSC_H7CR/liberty

ECC_CORE_DONE_DIR := $(ECC_BUILD_ROOT)/core
ECC_RTL_ROOT := $(ECC_BUILD_ROOT)/rtl
ECC_PROJECT_ROOT := $(ECC_BUILD_ROOT)/projects
ECC_QOR_REL := runs/default/Synthesis_yosys/report/post_synthesis/qor_summary.rpt
ECC_POWER_REL := runs/default/Synthesis_yosys/report/post_synthesis/power.rpt

$(ECC_ARCHIVE):
	@mkdir -p $(@D)
	curl -L --fail --retry 3 -C - -o $@ $(ECC_URL)

$(ECC_READY): $(ECC_ARCHIVE)
	@echo "$(ECC_ARCHIVE_SHA256)  $(ECC_ARCHIVE)" | sha256sum -c -
	@mkdir -p $(ECC_DIR)
	tar -xzf $(ECC_ARCHIVE) -C $(ECC_DIR)
	@test -x $(ECC_BIN)
	@touch $@

$(OSS_CAD_ARCHIVE):
	@mkdir -p $(@D)
	curl -L --fail --retry 3 -C - -o $@ $(OSS_CAD_URL)

$(OSS_CAD_READY): $(OSS_CAD_ARCHIVE)
	@echo "$(OSS_CAD_ARCHIVE_SHA256)  $(OSS_CAD_ARCHIVE)" | sha256sum -c -
	tar -xzf $(OSS_CAD_ARCHIVE) -C $(ECC_TOOLS_DIR)
	@test -x $(OSS_CAD_YOSYS)
	@touch $@

$(ICSPROUT55_READY):
	@if [ ! -d $(ICSPROUT55_DIR)/.git ]; then \
		git clone --depth 1 --branch unconfirmed \
			https://github.com/openecos-projects/icsprout55-pdk.git $(ICSPROUT55_DIR); \
	fi
	$(MAKE) -C $(ICSPROUT55_DIR) $(ICSPROUT55_LIB_TARGETS)
	@if [ -f $(ICSPROUT55_DIR)/patch/icg.diff ]; then \
		cd $(ICSPROUT55_DIR)/patch && bash patch.sh; \
	fi
	@touch $@

$(ECC_CORE_DONE_DIR)/%.done: $(CHISEL_SRCS)
	@mkdir -p $(@D)
	@rm -rf $(ECC_RTL_ROOT)/$*
	$(MILL) -i $(CHISEL_DESIGN).runMain Elaborate \
		--target-dir $(ECC_RTL_ROOT)/$* \
		--isa $* --platform core --reset-pc 0x$(ECC_CORE_RESET_PC)
	@touch $@

$(ECC_PROJECT_ROOT)/%/ecc.toml: $(ECC_CORE_DONE_DIR)/%.done $(ICSPROUT55_READY) scripts/gen-ecc-project.sh FORCE
	./scripts/gen-ecc-project.sh \
		$(ECC_PROJECT_ROOT)/$* ysyx_25100261_$* \
		$(ECC_RTL_ROOT)/$*/$* $(ICSPROUT55_DIR) \
		$(CPU_DESIGN_NAME) $(ECC_FREQ_MHZ)

$(ECC_PROJECT_ROOT)/%/$(ECC_QOR_REL): $(ECC_PROJECT_ROOT)/%/ecc.toml $(ECC_READY) $(OSS_CAD_READY) FORCE
	PATH="$(OSS_CAD_BIN):$$PATH" \
		YOSYS_SYNTH_STRATEGY="$(ECC_SYNTH_STRATEGY)" \
		$(ECC_BIN) run --overwrite --project $(ECC_PROJECT_ROOT)/$*

ecc-tools: $(ECC_READY) $(OSS_CAD_READY) $(ICSPROUT55_READY)

ecc-minirv: $(ECC_PROJECT_ROOT)/minirv/$(ECC_QOR_REL)

ecc-riscv32e: $(ECC_PROJECT_ROOT)/riscv32e/$(ECC_QOR_REL)

ecc-all: ecc-minirv ecc-riscv32e
	ECC_FREQ_MHZ=$(ECC_FREQ_MHZ) \
		./scripts/ecc-report.sh $(ECC_PROJECT_ROOT)/minirv $(ECC_PROJECT_ROOT)/riscv32e

# Compatibility alias for the old local target; ECC is now the only STA flow.
sta: ecc-riscv32e

FORCE:

.SECONDARY: $(addsuffix .done,$(addprefix $(ECC_CORE_DONE_DIR)/,$(ECC_ISAS))) \
	$(addsuffix /ecc.toml,$(addprefix $(ECC_PROJECT_ROOT)/,$(ECC_ISAS)))
.PHONY: ecc-tools ecc-minirv ecc-riscv32e ecc-all sta FORCE
