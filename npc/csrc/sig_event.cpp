#include "sig_event.hpp"
#include "common.hpp"

void HandShakeDetector::add(std::string pathWithoutValidOrReady) {
  auto pathValid = pathWithoutValidOrReady + "valid";
  auto pathReady = pathWithoutValidOrReady + "ready";
	spdlog::debug("HandShakeDetector: adding valid/ready pair: {}/{}",
								 pathValid, pathReady);
  vpiHandle hValid =
      vpi_handle_by_name(const_cast<PLI_BYTE8 *>(pathValid.c_str()), nullptr);
  vpiHandle hReady =
      vpi_handle_by_name(const_cast<PLI_BYTE8 *>(pathReady.c_str()), nullptr);
  if (hValid == nullptr || hReady == nullptr) {
    spdlog::error("HandShakeDetector: cannot find valid/ready handles for {}",
                  pathWithoutValidOrReady);
  }
  bus_list.emplace_back(hValid, hReady);
}
