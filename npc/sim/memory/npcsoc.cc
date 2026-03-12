#include "mem.hpp"

void init_mem_of_npc(void *img, const sim_config &cfg) {
  // spdlog::info("copy img to psram for sdb read");
  // g_sim_mem.psram.copy_from(img, img_size);
  spdlog::info("copy img to pmem for cpu core sim read");
  auto pmemDataPtr =
      get_dut()
          ->NPCTestSoC->vlSymsp->TOP__NPCTestSoC__npcDevices__mem__mem__mem_ext
          .Memory.data();
  memcpy(pmemDataPtr, img, cfg.img_size);

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
