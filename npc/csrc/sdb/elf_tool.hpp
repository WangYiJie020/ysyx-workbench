#pragma once
#include <elf.h>
#include <fstream>

class ElfHandler {
	void loadElf();
	void freeElf();
public:
		ElfHandler(std::fstream fs) {
			loadElf();
		}
		~ElfHandler() {
			freeElf();
		}

};
