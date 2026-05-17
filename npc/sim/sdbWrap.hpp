#pragma once
#include "sim.hpp"
#include <cstdint>
void sdb_init(uint32_t init_pc,uint32_t img_base, size_t img_size, const char* img_file_path, sim_setting setting);
// bool sbg_is_hitbadtrap();
void sdb_set_halt(int a0);
void sdb_dump_recent_info();

void sdb_skip_difftest_ref();

void sdb_memcpy_to_ref(uint32_t addr, std::span<uint8_t> data);

void sdb_exec(std::string_view cmd, bool *quit=nullptr);

int sdb_mainloop();
