#pragma once
#include "vpi_user.h"
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

struct ValidReadyBus {
  vpiHandle hValid;
  vpiHandle hReady;

  std::string description;

	size_t shake_count = 0;

  ValidReadyBus(vpiHandle hV, vpiHandle hR, std::string desc = "")
			: hValid(hV), hReady(hR), description(desc) {}

  bool shakeHappened() {
    s_vpi_value valValid, valReady;
    valValid.format = vpiIntVal;
    valReady.format = vpiIntVal;

    vpi_get_value(hValid, &valValid);
    vpi_get_value(hReady, &valReady);

    return (valValid.value.integer == 1) && (valReady.value.integer == 1);
  }
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

struct InstTypeCounter{
	size_t r_type = 0;
	size_t i_type = 0;
	size_t s_type = 0;
	size_t b_type = 0;
	size_t u_type = 0;
	size_t j_type = 0;

	void count(uint32_t inst);

};
