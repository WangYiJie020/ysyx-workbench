#include "mem.hpp"
#include <cstdint>

#ifdef SIM_ARCH_NPC

extern "C" void uart_putch(char ch);

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
      0x80000000u, 0xa0000000u, "pmem", 128 * 1024 * 1024);
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

extern "C" int pmem_read(int addr) {
  uint32_t data = 0;
  const uint32_t uaddr = static_cast<uint32_t>(addr);
  if (_pmem_ptr && _pmem_ptr->contains(uaddr)) {
    _pmem_ptr->read_word(uaddr & ~0x3u, data);
  }
  return static_cast<int>(data);
}

extern "C" void pmem_write(int addr, int data, char mask) {
  const uint32_t uaddr = static_cast<uint32_t>(addr);
  if (uaddr == 0x10000000u) {
    uart_putch(static_cast<char>(data));
    return;
  }
  assert(_pmem_ptr && _pmem_ptr->contains(uaddr));
  auto *bytes = _pmem_ptr->get_data_ptr_at(uaddr & ~0x3u);
  for (unsigned lane = 0; lane < 4; ++lane) {
    if ((static_cast<unsigned char>(mask) >> lane) & 1u) {
      bytes[lane] = (static_cast<uint32_t>(data) >> (lane * 8)) & 0xffu;
    }
  }
}

mem_region_data_span_vec get_mem_regions_need_init_difftest(){
	return {};
}
#endif
