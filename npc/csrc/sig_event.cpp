#include "sig_event.hpp"
#include <spdlog/sinks/stdout_color_sinks.h>

using std::move;

auto _FullPath(const std::string &pathWithoutValidOrReady,
               const std::string &suffix = "") {
  return cpu_vpi_path_prefix() + pathWithoutValidOrReady + suffix;
}
auto _DebugPath(const std::string &pathWithoutValidOrReady,
                const std::string &suffix = "") {
  return "`cpu." + pathWithoutValidOrReady + suffix;
}

void set_logger_pattern_with_simtime(std::shared_ptr<spdlog::logger> logger);
HandShakeDetector::HandShakeDetector() {
  logger = spdlog::stdout_color_mt("HandShakeDetector");
}
void HandShakeDetector::init() {
	set_logger_pattern_with_simtime(logger);
	logger->set_level(spdlog::level::info);
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
  auto hValid = SignalHandle(barePath + "valid");
	auto hReady = SignalHandle(barePath + "ready");

  spdlog::info("added watch for channel {} ({})", _DebugPath(barePath), description);
  bus_list.emplace_back(std::move(hValid), std::move(hReady), description);
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

void InstTypeCounter::init(){
	logger = spdlog::stdout_color_mt("InstTypeCounter");
	set_logger_pattern_with_simtime(logger);
	logger->set_level(spdlog::level::info);
	hInstType = SignalHandle("idu.iinfo_dec.io_out_typ");
	hInstFmt = SignalHandle("idu.iinfo_dec.io_out_fmt");
}
