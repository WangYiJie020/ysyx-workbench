#include "sig_event.hpp"
#include <spdlog/sinks/stdout_color_sinks.h>


void set_logger_pattern_with_simtime(std::shared_ptr<spdlog::logger> logger);
HandShakeDetector::HandShakeDetector() {
  logger = spdlog::stdout_color_mt("HandShakeDetector");
}
void HandShakeDetector::init() {
	set_logger_pattern_with_simtime(logger);
	logger->set_level(spdlog::level::info);
}

auto _FullPath(const std::string &pathWithoutValidOrReady,
               const std::string &suffix = "") {
  return cpu_vpi_path_prefix() + pathWithoutValidOrReady + suffix;
}
auto _DebugPath(const std::string &pathWithoutValidOrReady,
                const std::string &suffix = "") {
  return "`cpu." + pathWithoutValidOrReady + suffix;
}

SignalHandle::SignalHandle(std::string barePath) {
	handle = vpi_handle_by_name(
			const_cast<PLI_BYTE8 *>(_FullPath(barePath).c_str()), nullptr);
	if (!handle) {
		spdlog::error("cannot find signal at path {}", _DebugPath(barePath));
	}
}

void HandShakeDetector::add(std::string barePath, std::string description) {
  auto pathValid = _FullPath(barePath, "valid");
  auto pathReady = _FullPath(barePath, "ready");
  spdlog::debug("adding valid/ready pair: {}/{}", pathValid, pathReady);
  vpiHandle hValid =
      vpi_handle_by_name(const_cast<PLI_BYTE8 *>(pathValid.c_str()), nullptr);
  if (!hValid) {
    spdlog::error("cannot find valid signal at path {}",
                  _DebugPath(barePath, "valid"));
  }
  vpiHandle hReady =
      vpi_handle_by_name(const_cast<PLI_BYTE8 *>(pathReady.c_str()), nullptr);
  if (!hReady) {
    spdlog::error("cannot find ready signal at path {}",
                  _DebugPath(barePath, "ready"));
  }

  spdlog::info("added watch for channel {} ({})", _DebugPath(barePath), description);
  bus_list.emplace_back(hValid, hReady, description);
}

bool ValidReadyBus::shakeHappened() {
    s_vpi_value valValid, valReady;
    valValid.format = vpiIntVal;
    valReady.format = vpiIntVal;

    vpi_get_value(hValid.handle, &valValid);
    vpi_get_value(hReady.handle, &valReady);

    return (valValid.value.integer == 1) && (valReady.value.integer == 1);
  }

void HandShakeDetector::checkAndCountAll() {
	for(auto &bus : bus_list){
		if(bus.shakeHappened()){
			bus.shake_count++;
			logger->trace("Handshake happened on {} (total count {})",
			              _DebugPath(bus.description), bus.shake_count);
		}
	}
}
