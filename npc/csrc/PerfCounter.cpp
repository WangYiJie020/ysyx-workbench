#include "PerfCounter.hpp"
#include "sim.hpp"

#include <tabulate/table.hpp>

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

const char *EXUPerfCounter::nameOfTyp(int type) {
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
const char *EXUPerfCounter::nameOfFmt(int fmt) {
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

    instCountOfTyp[type]++;
    instCountOfFmt[fmt]++;

    auto instEndCycle = sim_get_cycle();
    auto instCycles = instEndCycle - instStartCycle;
    totalCycleOfTyp[type] += instCycles;
    totalCycleOfFmt[fmt] += instCycles;

    logger->trace("inst executed: type {} fmt {} cycles {}", nameOfTyp(type),
                  nameOfFmt(fmt), instCycles);
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

void EXUPerfCounter::_dump(size_t *instCnts, size_t *cycCnts, size_t num,
                           const char *(*nameFunc)(int)) {
  using namespace tabulate;
  Table t;
  t.add_row({"Category", "Inst Count", "Inst %", "Cycle Count", "Cycle %",
                 "Avg CPI"});

  size_t totalInsts = std::accumulate(instCnts, instCnts + num, 0ull);
  size_t totalCyc = std::accumulate(cycCnts, cycCnts + num, 0ull);

  for (size_t i = 0; i < num; i++) {
    auto name = nameFunc((int)i);
    auto instCount = instCnts[i];
    auto cycleCount = cycCnts[i];

    double instPerc = totalInsts == 0
                          ? NAN
                          : ((double)instCount / (double)totalInsts) * 100.0;
    double cyclePerc =
        totalCyc == 0 ? NAN : ((double)cycleCount / (double)totalCyc) * 100.0;

    double avgCPI =
        instCount == 0 ? NAN : (double)cycleCount / (double)instCount;

    t.add_row(RowStream{} << name << instCount << instPerc << cycleCount
                              << cyclePerc << avgCPI);
  }


	t.format().font_align(FontAlign::right);

	t.column(0).format().font_align(FontAlign::center);

	t.row(0).format().font_align(FontAlign::center);

	t.row(1).format().hide_border();

	std::cout << t << std::endl;
}

void EXUPerfCounter::dumpStatistics() {
  spdlog::info(">instruction type counts:");

  spdlog::info("  by type:");
  _dump(instCountOfTyp, totalCycleOfTyp, TYPE_NUM, nameOfTyp);
  spdlog::info("  by format:");
  _dump(instCountOfFmt, totalCycleOfFmt, FMT_NUM, nameOfFmt);
}

HandShakeDetector handshake_detector;
EXUPerfCounter exu_counter;
AXI4PerfCounterManager axi4_perf_counters;
IFUStateCounter ifu_state_counter;

void initPerfCounters() {

  handshake_detector.init();
  handshake_detector.add("ifu.io_mem_r", "IFU fetch inst");
  handshake_detector.add("exu.io_mem_r", "EXU load data");
  handshake_detector.add("exu.alu.io_out_", "EXU calc");
  handshake_detector.add("idu.io_out_", "IDU decode inst");

  axi4_perf_counters.addRead("exu.io_mem", "EXU load data");
  axi4_perf_counters.addWrite("exu.io_mem", "EXU store data");

  axi4_perf_counters.addRead("ifu.io_mem", "IFU fetch inst");

  exu_counter.bind("idu");
  ifu_state_counter.bind("ifu");
}

void updatePerfCounters() {
  handshake_detector.checkAndCountAll();
  exu_counter.update();
  axi4_perf_counters.updateAll();
  ifu_state_counter.update();
}
void dumpPerfCountersStatistics() {
  auto cycle_count = sim_get_cycle();
  auto inst_count = sim_get_inst_count();
  spdlog::info("simulation statistics:");
  fmt::println(">cycle and instruction counts:");
  fmt::println("  total cycle count: {}", cycle_count);
  fmt::println("  total instruction count: {}", inst_count);
  if (cycle_count == 0) {
    spdlog::warn("cycle count is 0, cannot calc IPC");
  } else {
    double ipc = (double)inst_count / (double)cycle_count;
    fmt::println("  IPC: {:.4f}", ipc);
  }
  if (inst_count == 0) {
    spdlog::warn("no instruction executed, cannot calc CPI");
  } else {
    double cpi = (double)cycle_count / (double)inst_count;
    fmt::println("  CPI: {:.4f}", cpi);
  }

  spdlog::info(">handshake counts:");
  for (auto &e : handshake_detector.bus_list) {
    e.dumpStatus();
  }

  ifu_state_counter.dumpStatistics();

  axi4_perf_counters.dumpAllStatistics();
}
