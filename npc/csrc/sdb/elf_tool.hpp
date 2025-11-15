#pragma once
#include <fstream>
#include <elf.h>
#include <vector>
#include <string>

class ElfHandler {
public:
	struct FuncSym {
		uint32_t addr;
		uint32_t size;
		std::string_view name;
	};
private:
	std::fstream& _fs;
	std::vector<FuncSym> _func_syms;
	std::string _symstr_buf;
	void loadElf();

	void _ensure_frd(void* ptr, size_t siz);
	std::string create_strbuf(const Elf32_Shdr& shdr);
public:

	ElfHandler(std::fstream& fs):_fs(fs) {
		loadElf();
	}
	void dump_func_syms();

};
