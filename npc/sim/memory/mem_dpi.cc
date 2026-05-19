#include "mem.hpp"
#include "mregion.hpp"

#include "../sim.hpp"

#include "../common.hpp"
#include <algorithm>
#include <cstdint>
#include <ranges>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/dup_filter_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <variant>

#define mem_regions get_mem_regions()

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
    spdlog::warn(
        "@{:08x} sim_read_vmem addr={:08x} no mapping region or read failed",
        sim_get_cpu_state()->pc, addr);
  return ok;
}

auto static _get_region(uint32_t addr) {
  mem_region_t *region = nullptr;
  for (auto &v : mem_regions) {
    if (std::visit([&](auto &r) { return r.contains(addr); }, v)) {
      region = &v;
      break;
    }
  }
  return region;
}

bool read_guest_mem(uint32_t addr, size_t n, std::vector<uint8_t> &data) {
  auto region = _get_region(addr);
  if (!region) {
    spdlog::warn("@{:08x} sim_read_vmem addr={:08x} no mapping region",
                 sim_get_cpu_state()->pc, addr);
    return false;
  }

  if (auto p = std::get_if<direct_mapped_mem>(region)) {
    auto data_ptr = p->get_data_ptr_at(addr);
    std::copy(data_ptr, data_ptr + n, std::back_inserter(data));
    return true;
  } else if (auto p = std::get_if<sdram_mem>(region)) {
    if (addr % 4 != 0 || n % 4 != 0) {
      spdlog::warn("sim_read_vmem vec from dram not supported word unaligned "
                   "addr {:08x}, pc={:08x}, size {}",
                   addr, sim_get_cpu_state()->pc, n);
      return false;
    }

    uint32_t word;
    uint8_t *byte_ptr = (uint8_t *)&word;
    for (size_t i = 0; i < n; i+=4) {
      p->read_word(addr + i, word);
      data.insert(data.end(), byte_ptr, byte_ptr + 4);
    }
    return true;
  } else {
    spdlog::warn("sim_read_vmem vec from unknown region type, addr {:08x}, "
                 "pc={:08x}, size {}",
                 addr, sim_get_cpu_state()->pc, n);
    return false;
  }
  return false;
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

bool write_guest_mem(uint32_t addr, const std::vector<uint8_t> &data) {
  auto region = _get_region(addr);
  if (!region) {
    spdlog::warn("sim_write_vmem vec addr={:08x} no mapping region", addr);
    return false;
  }

  if (auto p = std::get_if<direct_mapped_mem>(region)) {
    auto data_ptr = p->get_data_ptr_at(addr);
    std::copy(data.begin(), data.end(), data_ptr);
    return true;
  } else if (auto p = std::get_if<sdram_mem>(region)) {
    if (addr % 4 != 0 || data.size() % 4 != 0) {
      spdlog::warn("sim_write_vmem vec to dram not supported word unaligned "
                   "addr {:08x}, size {}",
                   addr, data.size());
      return false;
    }

    for (size_t i = 0; i < data.size(); i += 4) {
      uint32_t word = *(uint32_t *)&data[i];
      p->write_word(addr + i, word);
      if (i < 16) {
        spdlog::trace("sim_write_vmem to dram addr {:08x} <= {:08x} [{:02x} "
                      "{:02x} {:02x} {:02x}]",
                      addr + i, word, data[i], data[i + 1], data[i + 2],
                      data[i + 3]);
      }
    }
    return true;
  } else {
    spdlog::warn("sim_write_vmem vec to unknown region type, addr {:08x}, "
                 "size {}",
                 addr, data.size());
    return false;
  }
}
