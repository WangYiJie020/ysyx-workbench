REPO_PATH = repo
REPO_URL = https://github.com/WangYiJie020/ysyx-workbench-nemu-inst-gen.git
REPO_COMMIT = bedc48079ed8ed1e27baa9147aab86e25ad93461
REPO_REVISION = $(REPO_PATH)/.git/revision-$(REPO_COMMIT)

ifeq ($(NO_VENV),1)
VENV_PATH = /usr/local
else
VENV_PATH = $(abspath $(REPO_PATH)/.venv)
endif
VENV_CREATED = $(VENV_PATH)/bin/python
VENV_DONE = ./venv.done

OUT_DIR = $(abspath ./build)
$(shell mkdir -p $(OUT_DIR))

RISCV_UNIFIED_DB_GIT_DIR ?= $(abspath ../../../npc/deps/riscv-unified-db)
RISCV_UNIFIED_DB_COMMIT = d91b7d4ca6bfeaf1afab3af521a266dc5b92e4c7
RISCV_UNIFIED_DB_REVISION = $(RISCV_UNIFIED_DB_GIT_DIR)/.git/revision-$(RISCV_UNIFIED_DB_COMMIT)

default: $(REPO_REVISION) $(RISCV_UNIFIED_DB_REVISION) $(VENV_DONE)
	$(MAKE) -B -C $(REPO_PATH) VENV=$(VENV_PATH) BUILD_DIR=$(OUT_DIR) RISCV_UNIFIED_DB_GIT_DIR=$(RISCV_UNIFIED_DB_GIT_DIR)

$(REPO_PATH)/.git:
	git clone --no-checkout $(REPO_URL) $(REPO_PATH)

$(REPO_REVISION): $(REPO_PATH)/.git
	git -C $(REPO_PATH) fetch --depth=1 origin $(REPO_COMMIT)
	git -C $(REPO_PATH) checkout --detach FETCH_HEAD
	touch $@

$(RISCV_UNIFIED_DB_REVISION):
	git -C $(RISCV_UNIFIED_DB_GIT_DIR) fetch --depth=1 origin $(RISCV_UNIFIED_DB_COMMIT)
	git -C $(RISCV_UNIFIED_DB_GIT_DIR) checkout --detach FETCH_HEAD
	touch $@

$(VENV_CREATED):
	python3 -m venv $(VENV_PATH)

$(VENV_DONE): $(REPO_REVISION) $(VENV_CREATED)
	$(VENV_PATH)/bin/pip install -r $(REPO_PATH)/requirements.txt
	touch $@

clean:
	$(MAKE) -C $(REPO_PATH) clean

.PHONY: default clean
