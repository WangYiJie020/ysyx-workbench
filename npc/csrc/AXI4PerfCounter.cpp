#include "PerfCounter.hpp"
using namespace _PerfCtrImp;

void AXI4CounterBase::init_logger() {
  assert(!name.empty());
  logger = spdlog::stdout_color_mt(name);
  set_logger_pattern_with_simtime(logger);
  logger->set_level(spdlog::level::info);
}

void AXI4CounterBase::dumpStatisticsTitle(){
	fmt::println("  {:18} : {:>8} {:>10} {:>8} {:>8}",
							 "name", "txns", "cycles", "avg_lat", "max_lat");
}
void AXI4CounterBase::dumpStatistics() {
  // name : total_txns total_cycles avg_latency max_latency max_time_beg
  // max_time_end
  double avg_latency = transaction_count == 0 ? NAN
                                              : (double)total_latency_cycles /
                                                    (double)transaction_count;
  fmt::println("  {:18} : {:>8} {:>10} {:>8.2f} {:>8} (at sim time {} to {})", name,
               transaction_count, total_latency_cycles, avg_latency,
               maxRecord.cycles, maxRecord.startTime, maxRecord.endTime);
}

void AXI4ReadPerfCounter::bind(std::string channelPath) {
  hARValid = SignalHandle(channelPath + "_arvalid");
  hARReady = SignalHandle(channelPath + "_arready");
  hRValid = SignalHandle(channelPath + "_rvalid");
  hRReady = SignalHandle(channelPath + "_rready");
}

void AXI4ReadPerfCounter::update() {
  auto sim_time = sim_get_time();
  switch (state) {
  case IDLE: {
    if (hARValid.getUint32Value() == 1) {
      state = WAIT_DATA;
      transaction_count++;
      currentRecord.startTime = sim_time;
      currentRecord.cycles = 0;
      logger->trace("ARVALID high, starting transaction {}", transaction_count);
    }
    break;
  }
  case WAIT_DATA: {
    currentRecord.cycles++;
    if (hRValid.getUint32Value() == 1 && hRReady.getUint32Value() == 1) {
      // handshake happened
      currentRecord.endTime = sim_time;
      total_latency_cycles += currentRecord.cycles;
      if (currentRecord.cycles > maxRecord.cycles) {
        maxRecord = currentRecord;
      }
      logger->trace(
          "RVALID & RREADY handshake for transaction {} after {} cycles",
          transaction_count, currentRecord.cycles);
      state = IDLE;
    }
    break;
  }
  }
}
void AXI4WritePerfCounter::bind(std::string channelPath) {
  hAWValid = SignalHandle(channelPath + "_awvalid");
  hAWReady = SignalHandle(channelPath + "_awready");
  hWValid = SignalHandle(channelPath + "_wvalid");
  hWReady = SignalHandle(channelPath + "_wready");
  hBValid = SignalHandle(channelPath + "_bvalid");
  hBReady = SignalHandle(channelPath + "_bready");
}
void AXI4WritePerfCounter::update() {
  auto sim_time = sim_get_time();
  switch (state) {
  case IDLE: {
    if (hAWValid.getUint32Value() == 1) {
      state = WAIT_RESP;
      transaction_count++;
      currentRecord.startTime = sim_time;
      currentRecord.cycles = 0;
      logger->trace("AWVALID high, starting transaction {}", transaction_count);
    }
    break;
  }
  case WAIT_RESP: {
    currentRecord.cycles++;
    if (hBValid.getUint32Value() == 1 && hBReady.getUint32Value() == 1) {
      // handshake happened
      currentRecord.endTime = sim_time;
      total_latency_cycles += currentRecord.cycles;
      if (currentRecord.cycles > maxRecord.cycles) {
        maxRecord = currentRecord;
      }
      logger->trace(
          "BVALID & BREADY handshake for transaction {} after {} cycles",
          transaction_count, currentRecord.cycles);
      state = IDLE;
    }
    break;
  }
  }
}

void AXI4PerfCounterManager::updateAll() {
  for (auto &ctr : rdCounters) {
    ctr.update();
  }
  for (auto &ctr : wrCounters) {
    ctr.update();
  }
}
void AXI4PerfCounterManager::addRead(std::string channelPath,
                                     std::string name) {
  AXI4ReadPerfCounter ctr;
  ctr.name = name;
  ctr.bind(channelPath);
  ctr.init_logger();
  spdlog::debug("added AXI4 read perf counter '{}' for channel '{}'", name,
                _DebugPath(channelPath));
  rdCounters.push_back(std::move(ctr));
}
void AXI4PerfCounterManager::addWrite(std::string channelPath,
                                      std::string name) {
  AXI4WritePerfCounter ctr;
  ctr.name = name;
  ctr.bind(channelPath);
  ctr.init_logger();
  spdlog::debug("added AXI4 write perf counter '{}' for channel '{}'", name,
                _DebugPath(channelPath));
  wrCounters.push_back(std::move(ctr));
}

void AXI4PerfCounterManager::dumpAllStatistics() {
  spdlog::info("AXI4 Performance Counters Statistics:");
  fmt::println(">AXI4 Read Counters:");
	AXI4CounterBase::dumpStatisticsTitle();
  for (auto &ctr : rdCounters) {
    ctr.dumpStatistics();
  }
  fmt::println(">AXI4 Write Counters:");
	AXI4CounterBase::dumpStatisticsTitle();
  for (auto &ctr : wrCounters) {
    ctr.dumpStatistics();
  }
}
