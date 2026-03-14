#include "mem.hpp"
#include <cstdint>

#ifdef SIM_ARCH_NPC

static std::shared_ptr<direct_mapped_mem> _pmem_ptr;

mem_region_group_t &get_mem_regions() {
  static mem_region_group_t mem_regions;
  if (mem_regions.empty()) {
    if (_pmem_ptr) {
      mem_regions.push_back(*_pmem_ptr);
    } else {
      spdlog::warn("try to get mem regions of npc before init, returning empty "
                   "region list");
    }
  }
  return mem_regions;
}

void init_mem(void *img, const sim_config &cfg) {
  spdlog::info("copy img to pmem of npc");

  _pmem_ptr = std::make_shared<direct_mapped_mem>(
      0x80000000u, 0xa0000000u, "pmem", 128 * 1024 * 1024,
      get_dut()
          ->TestSoC->vlSymsp->TOP__TestSoC__devices__mem__mem__mem_ext
          .Memory.data());
  _pmem_ptr->copy_from(img, cfg.img_size);

  // auto pmemDataPtr = memcpy(pmemDataPtr, img, cfg.img_size);

  //   // memset the ASAN shadow memory to zero
  //   // instead at trm_init to optimise the init time
  // #define ASAN_SHADOW_MEMORY_START 0x7000000
  // #define ASAN_SHADOW_MEMORY_SIZE 0x1000000
  //
  //   auto psram_data = pmemDataPtr;
  //   memset((uint8_t *)psram_data + ASAN_SHADOW_MEMORY_START, 0,
  //          ASAN_SHADOW_MEMORY_SIZE);
  //   spdlog::info("ASAN shadow memory in psram zeroed");
}

mem_region_data_span_vec get_mem_regions_need_init_difftest(){
	return {};
}
#endif
