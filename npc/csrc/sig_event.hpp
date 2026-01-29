#pragma once
#include "vpi_user.h"
#include <vector>
#include <verilated.h>
#include <verilated_vpi.h>

#include "vsrc.hpp"

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

  ValidReadyBus(vpiHandle hV, vpiHandle hR) : hValid(hV), hReady(hR) {}

  bool shakeHappened() {
    s_vpi_value valValid, valReady;
    valValid.format = vpiIntVal;
    valReady.format = vpiIntVal;

    vpi_get_value(hValid, &valValid);
    vpi_get_value(hReady, &valReady);

    return (valValid.value.integer == 1) && (valReady.value.integer == 1);
  }
};

struct HandShakeDetector {
  std::vector<ValidReadyBus> bus_list;
  void add(std::string pathWithoutValidOrReady);
};
