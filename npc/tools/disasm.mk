LIBCAPSTONE = tools/capstone/repo/libcapstone.so.5
INC_PATH += $(abspath tools/capstone/repo/include)

csrc/sdb/disasm.c: $(LIBCAPSTONE)

$(LIBCAPSTONE):
	$(MAKE) -C tools/capstone
