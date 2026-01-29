#include "sig_event.hpp"
#include <spdlog/sinks/stdout_color_sinks.h>

HandShakeDetector::HandShakeDetector() {
  logger = spdlog::stdout_color_mt("HandShakeDetector");
  logger->set_level(spdlog::level::debug);
}

auto _FullPath(const std::string &pathWithoutValidOrReady,
               const std::string &suffix = "") {
  return cpu_vpi_path_prefix() + pathWithoutValidOrReady + suffix;
}
auto _DebugPath(const std::string &pathWithoutValidOrReady,
                const std::string &suffix = "") {
  return "`cpu." + pathWithoutValidOrReady + suffix;
}

void HandShakeDetector::add(std::string barePath, std::string description) {
  auto pathValid = _FullPath(barePath, "valid");
  auto pathReady = _FullPath(barePath, "ready");
  logger->debug("adding valid/ready pair: {}/{}", pathValid, pathReady);
  vpiHandle hValid =
      vpi_handle_by_name(const_cast<PLI_BYTE8 *>(pathValid.c_str()), nullptr);
  if (!hValid) {
    logger->error("cannot find valid signal at path {}",
                  _DebugPath(barePath, "valid"));
  }
  vpiHandle hReady =
      vpi_handle_by_name(const_cast<PLI_BYTE8 *>(pathReady.c_str()), nullptr);
  if (!hReady) {
    logger->error("cannot find ready signal at path {}",
                  _DebugPath(barePath, "ready"));
  }

  logger->info("added watch for channel {} ({})", barePath, description);
  bus_list.emplace_back(hValid, hReady, description);
}
