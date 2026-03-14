#include "mem.hpp"
#include <cstdint>
#include <filesystem>

namespace fs = std::filesystem;

#ifdef SIM_ARCH_JYD

static std::shared_ptr<direct_mapped_mem> irom_ptr, dram_ptr;

mem_region_group_t &get_mem_regions() {
  static mem_region_group_t mem_regions;
  if (mem_regions.empty()) {
    if (irom_ptr && dram_ptr) {
      mem_regions.push_back(*irom_ptr);
      mem_regions.push_back(*dram_ptr);
    } else {
      spdlog::warn(
          "try to get mem regions of jydsoc before init, returning empty "
          "region list");
    }
  }
  return mem_regions;
}

void init_mem(void *img, const sim_config &cfg) {
  fs::path img_path(cfg.img_file_path);
  auto dram_path = img_path;
  dram_path.replace_extension(".data.bin");
  if (!fs::exists(dram_path)) {
    spdlog::error("DRAM data file {} does not exist, cannot init DRAM content",
                  dram_path.string());
    exit(1);
  }

  irom_ptr = std::make_shared<direct_mapped_mem>(
      0x80000000u, 0x80004000u, "irom", 16 * 1024,
      get_dut()
          ->TestSoC->vlSymsp->TOP__TestSoC__devices__irom__mem__mem_ext.Memory
          .data());
  dram_ptr = std::make_shared<direct_mapped_mem>(
      0x80100000u, 0x80140000u, "dram", 256 * 1024,
      get_dut()
          ->TestSoC->vlSymsp->TOP__TestSoC__devices__dram__mem__mem_ext.Memory
          .data());
}

#endif
