ifneq ($(filter gdb-server,$(MAKECMDGOALS)),)

# 加入 -O0
CFLAGS += -O0 -g
CXXFLAGS += -O0 -g

else

CFLAGS += -O3
CXXFLAGS += -O3

endif

gdb-server: insert-arg
	@echo "CFLAGS: $(CFLAGS)"
	@cd $(YSYX_HOME)/renode && ./run-gdb.sh server $(IMAGE).elf

gdb-client:
	@cd $(YSYX_HOME)/renode && ./run-gdb.sh gdb $(IMAGE).elf
