#include "sig_event.hpp"
#include "common.hpp"

void HandShakeDetector::add(std::string pathWithoutValidOrReady) {
  auto pathValid = cpu_vpi_path_prefix() + pathWithoutValidOrReady + "valid";
  auto pathReady = cpu_vpi_path_prefix() + pathWithoutValidOrReady + "ready";
	spdlog::debug("HandShakeDetector: adding valid/ready pair: {}/{}",
								 pathValid, pathReady);
  vpiHandle hValid =
      vpi_handle_by_name(const_cast<PLI_BYTE8 *>(pathValid.c_str()), nullptr);
	if(!hValid){
		spdlog::error("HandShakeDetector: cannot find valid signal at path {}", pathValid);
	}
  vpiHandle hReady =
      vpi_handle_by_name(const_cast<PLI_BYTE8 *>(pathReady.c_str()), nullptr);
	if(!hReady){
		spdlog::error("HandShakeDetector: cannot find ready signal at path {}", pathReady);
	}

  bus_list.emplace_back(hValid, hReady);
}
