#include "sig_event.hpp"
#include "sim.hpp"
#include "spdlog/common.h"

using namespace _PerfCtrImp;

HandShakeDetector::HandShakeDetector() {
  logger = spdlog::stdout_color_mt("HandShakeDetector");
}
void HandShakeDetector::init() {
  set_logger_pattern_with_simtime(logger);
  logger->set_level(spdlog::level::info);
}

SignalHandle::SignalHandle(std::string barePath) {
  // spdlog::debug("resolving signal path: {}", _DebugPath(barePath));
  handle = vpi_handle_by_name(
      const_cast<PLI_BYTE8 *>(_FullPath(barePath).c_str()), nullptr);
  if (!handle) {
    spdlog::error("cannot find signal at path {}", _DebugPath(barePath));
  }
}

HandShakeDetector::ValidReadyBus &
HandShakeDetector::add(std::string barePath, std::string description,
                       callback_t onShake) {
  auto pathValid = _FullPath(barePath, "valid");
  auto pathReady = _FullPath(barePath, "ready");
  spdlog::trace("adding valid/ready pair: {}/{}", pathValid, pathReady);
  auto hValid = SignalHandle(barePath + "valid");
  auto hReady = SignalHandle(barePath + "ready");

  spdlog::info("added watch for channel {} ({})", _DebugPath(barePath),
               description);
  bus_list.emplace_back(ValidReadyBus{
      .hValid = std::move(hValid),
      .hReady = std::move(hReady),
      .description = description,
      .onShakeCallback = onShake,
  });
  return bus_list.back();
}

bool HandShakeDetector::ValidReadyBus::shakeHappened() {
  return hValid.getUint32Value() == 1 && hReady.getUint32Value() == 1;
}
void HandShakeDetector::ValidReadyBus::dumpStatus() {
  fmt::println("  {:18} happened {:>7} times (freq {:.4f})", description,
               shake_count, (double)shake_count / (double)sim_get_cycle());
}

void HandShakeDetector::checkAndCountAll() {
  for (auto &bus : bus_list) {
    if (bus.shakeHappened()) {
      bus.shake_count++;
      logger->trace("Handshake happened on {} (total count {})",
                    _DebugPath(bus.description), bus.shake_count);
      if (bus.onShakeCallback) {
        bus.onShakeCallback();
      }
    }
  }
}

const char *EXUPerfCounter::name_of_type(InstType type) {
  static const char *type_names[] = {
      "branch", "arithmetic", "load",  "store",  "jalr",
      "jal",    "lui",        "auipc", "system",
  };
  if (type < sizeof(type_names) / sizeof(type_names[0])) {
    return type_names[type];
  } else {
    return "unknown";
  }
}
const char *EXUPerfCounter::name_of_fmt(InstFmt fmt) {
  static const char *fmt_names[] = {
      "I_TYPE", "R_TYPE", "S_TYPE", "U_TYPE", "J_TYPE", "B_TYPE",
  };
  if (fmt < sizeof(fmt_names) / sizeof(fmt_names[0])) {
    return fmt_names[fmt];
  } else {
    return "unknown";
  }
}
void EXUPerfCounter::bind(std::string path) {
  logger = spdlog::stdout_color_mt("InstTypeCounter");
  set_logger_pattern_with_simtime(logger);
  logger->set_level(spdlog::level::info);
  hInstType = SignalHandle(path + ".io_out_bits_info_typ");
  hInstFmt = SignalHandle(path + ".io_out_bits_info_fmt");
  hOutValid = SignalHandle(path + ".io_out_valid");
  hOutReady = SignalHandle(path + ".io_out_ready");
}
void EXUPerfCounter::update() {

	bool isOutValidRasingEdge =
			(!lastCycOutValid && hOutValid.getUint32Value() == 1);
	lastCycOutValid = (hOutValid.getUint32Value() == 1);

  //
  // For history reason the timing is wired
  //
  // a inst start execution approximately when out_valid rises
  // (TODO: figure out the exact timing)
  // and ends when out_ready highs
  //

	if (isOutValidRasingEdge) {
		instStartCycle = sim_get_cycle();
	}

	if (hOutReady.getUint32Value() == 1 && hOutValid.getUint32Value() == 1) {
		// instruction finished execution
		InstType type = (InstType)hInstType.getUint32Value();
		InstFmt fmt = (InstFmt)hInstFmt.getUint32Value();

		assert(isValidType(type));
		assert(isValidFmt(fmt));

		type_count[type]++;
		fmt_count[fmt]++;

		auto instEndCycle = sim_get_cycle();
		auto instCycles = instEndCycle - instStartCycle;
		tot_cycle_of_type[type] += instCycles;
		tot_cycle_of_fmt[fmt] += instCycles;

		logger->trace("inst executed: type {} fmt {} cycles {}", name_of_type(type),
									name_of_fmt(fmt), instCycles);
	}
}

void IFUStateCounter::bind(std::string basePath) {
  hRValid = SignalHandle(basePath + ".io_mem_rvalid");
  hRReady = SignalHandle(basePath + ".io_mem_rready");

  hState = SignalHandle(basePath + ".fsm.state");
}
void IFUStateCounter::update() {
  bool fetchInstHappened =
      (hRReady.getUint32Value() == 1 && hRValid.getUint32Value() == 1);
  State s = (State)hState.getUint32Value();
  countOfState[s]++;
  if (fetchInstHappened) {
    totalFetchCount++;
  } else {
    countOfStateWhenNoFetch[s]++;
  }
}
static const char *_name_of_ifu_state(IFUStateCounter::State s) {
  const char *names[] = {
      "IDLE",
      "WAIT_INST",
      "WAIT_LATER",
  };
  return names[(size_t)s];
}
void IFUStateCounter::dumpStatistics() {
  spdlog::info("IFU State Counter Statistics:");
  fmt::println("  total instruction fetch count: {}", totalFetchCount);
  fmt::println("  state statistics:");
  fmt::println("    {:10} : {:<18} {:<18}", "state", "count",
               "count[exclu fetch]");
  auto totCycles = sim_get_cycle();
  for (size_t i = 0; i < STATE_NUM; i++) {
    double perc = totCycles == 0
                      ? NAN
                      : ((double)countOfState[i] / (double)totCycles) * 100.0;
    double percNoFetch =
        totCycles == 0
            ? NAN
            : ((double)countOfStateWhenNoFetch[i] / (double)totCycles) * 100.0;

    fmt::println("    {:10} : {:>8} ({:6.3f}%) {:>8} ({:6.3f}%)",
                 _name_of_ifu_state((State)i), countOfState[i], perc,
                 countOfStateWhenNoFetch[i], percNoFetch);
  }
}
