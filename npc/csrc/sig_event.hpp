#pragma once
#include "vpi_user.h"
#include <cstddef>
#include <vector>
#include <verilated.h>
#include <verilated_vpi.h>

#include "vsrc.hpp"

#include "common.hpp"

inline const std::string &cpu_vpi_path_prefix() {
  static std::string prefix;
  if (prefix.empty()) {
    prefix = std::string("TOP.") + std::string(_STR(TOP_NAME)).substr(1);
    prefix += ".asic.cpu.cpu.";
  }
  return prefix;
}

struct SignalHandle {
  vpiHandle handle;
  ~SignalHandle() {
    if (handle) {
      vpi_release_handle(handle);
    }
  }
  SignalHandle() : handle(nullptr) {}
  SignalHandle(vpiHandle h) : handle(h) {}

  SignalHandle(const SignalHandle &) = delete;
  SignalHandle &operator=(const SignalHandle &) = delete;
  SignalHandle(SignalHandle &&other) : handle(other.handle) {
    other.handle = nullptr;
  }
  SignalHandle &operator=(SignalHandle &&other) {
    if (this != &other) {
      if (handle) {
        vpi_release_handle(handle);
      }
      handle = other.handle;
      other.handle = nullptr;
    }
    return *this;
  }

  SignalHandle(std::string barePath);
};

struct ValidReadyBus {
  SignalHandle hValid;
  SignalHandle hReady;

  std::string description;

  size_t shake_count = 0;

  ValidReadyBus(SignalHandle&& hV, SignalHandle&& hR, std::string desc="")
      : hValid(std::move(hV)), hReady(std::move(hR)), description(desc) {}

  bool shakeHappened();
};

class HandShakeDetector {
public:
  std::shared_ptr<spdlog::logger> logger;
  std::vector<ValidReadyBus> bus_list;

  HandShakeDetector();

  void init();
  void add(std::string pathWithoutValidOrReady, std::string description = "");

  void checkAndCountAll();
};

struct InstTypeCounter {
  // in common_def.scala
  //   val imm, reg, store, upper, jump, branch = Value
  //   val none, arithmetic, load, store, jalr, jal, lui, auipc, system =
  enum InstFmt { I_TYPE, R_TYPE, S_TYPE, U_TYPE, J_TYPE, B_TYPE, FMT_COUNT };
  enum InstType {
    none,
    arithmetic,
    load,
    store,
    jalr,
    jal,
    lui,
    auipc,
    system,
		TYPE_COUNT
  };
	size_t fmt_counts[FMT_COUNT] = {0};
	size_t type_counts[TYPE_COUNT] = {0};

	size_t tot_cycle_of_type[TYPE_COUNT] = {0};
	size_t tot_cycle_of_fmt[FMT_COUNT] = {0};

  SignalHandle hInstType;
	SignalHandle hInstFmt;

	std::shared_ptr<spdlog::logger> logger;

  void init();

  void count(uint32_t inst);
};
