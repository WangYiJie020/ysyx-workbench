#include "mregion.hpp"
#include "mem.hpp"

#include "../sim.hpp"

#include <spdlog/spdlog.h>
#include <variant>
#include <ranges>
#include <algorithm>

#define EXTERN_C extern "C"

#define DPI_LOG(lvl, fmt, ...)                                                 \
  do {                                                                         \
    auto _loger = spdlog::get(__func__);                                       \
    if (_loger)                                                                \
      _loger->lvl(fmt, ##__VA_ARGS__);                                         \
  } while (0)
#define DPI_TRACE(fmt, ...) DPI_LOG(trace, fmt, ##__VA_ARGS__)
#define DPI_ERROR(fmt, ...) DPI_LOG(error, fmt, ##__VA_ARGS__)
#define DPI_CRITICAL(fmt, ...) DPI_LOG(critical, fmt, ##__VA_ARGS__)

#define DPI_ASSERT(cond, fmt, ...)                                             \
  do {                                                                         \
    if (!(cond)) {                                                             \
      DPI_CRITICAL(fmt, ##__VA_ARGS__);                                        \
      assert(cond);                                                            \
    }                                                                          \
  } while (0)

struct {
  direct_mapped_mem mrom = {0x20000000u, 0x20010000u, "mrom"};
  direct_mapped_mem flash = {0x30000000u, 0x40000000u, "flash",
                             64 * 1024 * 1024};
  direct_mapped_mem psram = {0x80000000u, 0xa0000000u, "psram",
                             128 * 1024 * 1024};
  direct_mapped_mem sram = {0x0f000000u, 0x10000000u, "sram", 8 * 1024};

  sdram_mem sdram = {0xa0000000u, 0xb0000000u, "sdram"};
} g_sim_mem;

using mem_region_t = std::variant<direct_mapped_mem, sdram_mem>;
std::vector<mem_region_t> mem_regions = {g_sim_mem.mrom, g_sim_mem.flash,
                                         g_sim_mem.psram, g_sim_mem.sram,
                                         g_sim_mem.sdram};


EXTERN_C void mrom_read(int32_t addr, int32_t *data) {
	g_sim_mem.mrom.read_word(addr, (uint32_t &)(*data));
	DPI_TRACE("R addr={:08x} data={:08x}", addr, *data);
}

#define DEF_NOHIGH8_FORWARD_RD(region)                                         \
  EXTERN_C void region##_read(int32_t addr, int32_t *data) {                  \
    DPI_ASSERT((addr & 0xff000000) == 0,                                       \
               #region "_read addr={:08x} has non-zero high 8 bits", addr);    \
    addr += g_sim_mem.region.base();                                           \
    g_sim_mem.region.read_word(addr, (uint32_t &)(*data));                     \
    DPI_TRACE("R addr={:08x} data={:08x}", addr, *data);                       \
  }
// in spi
//   .addr({8'b0, in_paddr[23:2], 2'b0}),
// so the high 8 bits are ignored
// 0x3XXXXXXX -> 0x00XXXXXX
DEF_NOHIGH8_FORWARD_RD(flash)

DEF_NOHIGH8_FORWARD_RD(psram)
// compatible interface for npc core
extern "C" void pmem_read(int addr, int *data) {
  return psram_read(addr - g_sim_mem.psram.base(), data);
}

extern "C" void psram_write(int32_t addr, char strb8, int32_t data, int32_t *) {
  DPI_ASSERT((addr & 0xff000000) == 0,
             "psram_write addr={:08x} has non-zero high 8 bits", addr);
  // add back
  addr += g_sim_mem.psram.base();
  g_sim_mem.psram.write_word(addr, data, strb8);
  uint32_t newdata;
  g_sim_mem.psram.read_word(addr, newdata);

  DPI_TRACE("W addr={:08x} data={:08x} (strb {:02x}) newdata={:08x}", addr,
            (uint32_t)data, (uint32_t)strb8, newdata);
}
// compatible interface for npc core
extern "C" void pmem_write(int addr, int data, int mask) {
  uint8_t unaligned_part = addr & 0x3;
  uint32_t udata = ((uint32_t)data) >> (unaligned_part * 8);
  uint8_t umask = (mask >> unaligned_part) & 0xf;
  return psram_write(addr - g_sim_mem.psram.base(), umask, udata, nullptr);
}


extern "C" void sdram_read(char block, char bank, short row, short col,
                           short *data) {
	g_sim_mem.sdram.read_half(block, bank, row, col, (uint16_t &)(*data));
  DPI_TRACE("R bank={:02x} row={:04x} col={:04x} block={} data={:04x}", bank,
            row, col, (uint32_t)block, (uint16_t)*data);
}
extern "C" void sdram_write(char block, char bank, short row, short col,
                            short data, char mask) {
	uint16_t new_data;
	g_sim_mem.sdram.read_half(bank, row, col, block, new_data);
	
  // mask [0] = 0: write low byte
  // mask [1] = 0: write high byte
  if ((mask & 0x1) == 0) {
    new_data &= 0xff00;
    new_data |= (data & 0x00ff);
  }
  if ((mask & 0x2) == 0) {
    new_data &= 0x00ff;
    new_data |= (data & 0xff00);
  }

	g_sim_mem.sdram.write_half(block, bank, row, col, new_data);

  char human_friendly_mask[3] = {'-', '-', '\0'};
  if ((mask & 0x1) == 0)
    human_friendly_mask[1] = 'L';
  if ((mask & 0x2) == 0)
    human_friendly_mask[0] = 'H';

  DPI_TRACE("W bank={:02x} row={:04x} col={:04x} block={} "
            "data={:04x} mask={} newdata={:04x}",
            bank, row, col, (uint32_t)block, (uint16_t)data,
            human_friendly_mask, new_data);
}

extern "C" void sram_upd(int addr, int data, char mask){
	g_sim_mem.sram.write_word(addr, data, mask);
}

uint8_t *mem_guest_to_host(uint32_t addr) {
  for (auto &r : mem_regions) {
    auto res = std::visit(
        [&](auto &region) -> uint8_t * {
          if (region.contains(addr)) {
            return region.get_data_ptr_at(addr);
          }
          return nullptr;
        },
        r);
    if (res) {
      return res;
    }
  }
  spdlog::error("sim_guest_to_host addr={:08x} no mapping region", addr);
  return nullptr;
}

bool read_guest_mem(uint32_t addr, uint32_t *data) {
  bool ok = std::ranges::any_of(mem_regions, [&](auto &v) {
    return std::visit(
        [&](auto &r) { return r.contains(addr) && r.read_word(addr, *data); },
        v);
  });
  if (!ok)
    spdlog::warn("sim_read_vmem addr={:08x} no mapping region or read failed",
                 addr);
  return ok;
}
bool write_guest_mem(uint32_t addr, uint32_t data) {
  bool ok = std::ranges::any_of(mem_regions, [&](auto &v) {
    return std::visit(
        [&](auto &r) { return r.contains(addr) && r.write_word(addr, data); },
        v);
  });
  if (!ok)
    spdlog::warn("sim_write_vmem addr={:08x} no mapping region or write failed",
                 addr);
  return ok;
}


static void _copy_img(void *img, size_t img_size) {
  if (is_soc()) {
    spdlog::info("copy img to flash for soc sim");
    g_sim_mem.flash.copy_from(img, img_size);
  } else {
    spdlog::info("copy img to psram for cpu core sim");
    g_sim_mem.psram.copy_from(img, img_size);
  }
}
static void _fill_rams_uninit(bool zero_uninit_ram) {
  if (zero_uninit_ram) {
    if (is_soc()) {
      g_sim_mem.psram.fill(0);
    }
		g_sim_mem.sdram.fill(0);
    // memset(sdram_data, 0, sizeof(sdram_data));
  } else {
    if (is_soc()) {
      g_sim_mem.psram.fill(0xcc);
      spdlog::debug("psram_data filled with 0xcc");
    }
		g_sim_mem.sdram.fill(0xdd);
    // memset(sdram_data, 0xdd, sizeof(sdram_data));
    spdlog::debug("sdram_data filled with 0xdd");
  }
  spdlog::info("RAMs uninitialized area filled with {}",
               zero_uninit_ram ? "zeros" : "non-zero patterns");

  if (!is_soc()) {
    // memset the ASAN shadow memory to zero
    // instead at trm_init to optimise the init time
#define ASAN_SHADOW_MEMORY_START 0x7000000
#define ASAN_SHADOW_MEMORY_SIZE 0x1000000

    auto psram_data = g_sim_mem.psram.data;
    memset((uint8_t *)psram_data + ASAN_SHADOW_MEMORY_START, 0,
           ASAN_SHADOW_MEMORY_SIZE);
    spdlog::info("ASAN shadow memory in psram zeroed");
  }
}

void mem_init(void* img, size_t img_size, bool zero_uninit_ram) {
	_copy_img(img, img_size);
	_fill_rams_uninit(zero_uninit_ram);

	init_mem_logger();
}
