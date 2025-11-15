#include "elf_tool.hpp"

#include <elf.h>
#include <cstring>
#include <format>
#include <stdexcept>
#include <vector>
#include <string>
#include <ranges>
#include <algorithm>
#include <source_location>

using namespace std;
using namespace std::views;

void _check(bool cond, const string& msg,
		const source_location& loc = source_location::current()) {
	if (!cond) {
		throw runtime_error(format("{}:{}: ElfHandler: {}", loc.file_name(), loc.line(), msg));
	}
}

string ElfHandler::create_strbuf(const Elf32_Shdr& shdr) {
	_check(shdr.sh_type == SHT_STRTAB, "try read strtable on wrong header type");
	_fs.seekg(shdr.sh_offset, ios::beg);
	string buf(shdr.sh_size, '\0');
	_ensure_frd(buf.data(), shdr.sh_size);
	return buf;
}

void ElfHandler::_ensure_frd(void* ptr, size_t siz) {
	_check(_fs.read((char*)ptr, siz).gcount() == siz,
			format("read failed, expected {} bytes", siz));
}
void ElfHandler::loadElf() {
	// Read ELF header
	char e_ident[EI_NIDENT];
	_ensure_frd(e_ident, EI_NIDENT);

	_check(e_ident[EI_CLASS] == ELFCLASS32,"only support 32-bit ELF files"); 
	_check(memcmp(e_ident, ELFMAG, SELFMAG) == 0,"not a valid ELF file");

	Elf32_Ehdr hdr;
	_fs.seekg(0, ios::beg);
	_ensure_frd(&hdr, sizeof(hdr));

	// Read section headers
	_check(hdr.e_shstrndx < hdr.e_shnum,"invalid section header string table index");

	vector<Elf32_Shdr> shdrs(hdr.e_shnum);
	_fs.seekg(hdr.e_shoff, ios::beg);
	_ensure_frd(shdrs.data(), hdr.e_shnum * sizeof(Elf32_Shdr));

	// Find symbol table section
	auto it = ranges::find(shdrs, SHT_SYMTAB, &Elf32_Shdr::sh_type);
	_check(it != shdrs.end(),"symbol table section not found");
	const auto& sh_symtab = *it;

	// Read symbol table entries
	_check(sh_symtab.sh_entsize == sizeof(Elf32_Sym),
			"symbol table entry size mismatch");
	_check(sh_symtab.sh_size%sh_symtab.sh_entsize==0, 
			"symbol table size not multiple of entry size");

	size_t n_symbols = sh_symtab.sh_size / sh_symtab.sh_entsize;
	vector<Elf32_Sym> syms(n_symbols);

	_fs.seekg(sh_symtab.sh_offset, ios::beg);
	_ensure_frd(syms.data(), sh_symtab.sh_size);

	// Read symbol string table
	_check(sh_symtab.sh_link<hdr.e_shnum, "invalid symbol string table index");
	const auto& sh_symstrtab = shdrs[sh_symtab.sh_link];
	_symstr_buf = create_strbuf(sh_symstrtab);

	// Extract function symbols
	for(auto& s: syms) {
		if(ELF32_ST_TYPE(s.st_info) != STT_FUNC) continue;
		_func_syms.emplace_back(s.st_value, s.st_size,&_symstr_buf[s.st_name]);
	}

}

void ElfHandler::dump_func_syms() {
	for (const auto& f : _func_syms) {
		printf("FUNC: %08X %5d %s\n", f.addr, f.size, f.name.data());
	}
}
