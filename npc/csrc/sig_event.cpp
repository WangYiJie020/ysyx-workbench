#include "sig_event.hpp"
#include "common.hpp"

auto _FullPath(const std::string &pathWithoutValidOrReady,
               const std::string &suffix) {
  return cpu_vpi_path_prefix() + pathWithoutValidOrReady + suffix;
}
auto _DebugPath(const std::string &pathWithoutValidOrReady,
                const std::string &suffix) {
  return "(.cpu.)" + pathWithoutValidOrReady + suffix;
}

void HandShakeDetector::add(std::string barePath) {
  auto pathValid = _FullPath(barePath, "valid");
  auto pathReady = _FullPath(barePath, "ready");
  spdlog::debug("HandShakeDetector: adding valid/ready pair: {}/{}", pathValid,
                pathReady);
  vpiHandle hValid =
      vpi_handle_by_name(const_cast<PLI_BYTE8 *>(pathValid.c_str()), nullptr);
  if (!hValid) {
    spdlog::error("HandShakeDetector: cannot find valid signal at path {}",
                  _DebugPath(barePath, "valid"));
  }
  vpiHandle hReady =
      vpi_handle_by_name(const_cast<PLI_BYTE8 *>(pathReady.c_str()), nullptr);
  if (!hReady) {
    spdlog::error("HandShakeDetector: cannot find ready signal at path {}",
                  _DebugPath(barePath, "ready"));
  }

  bus_list.emplace_back(hValid, hReady);
}
