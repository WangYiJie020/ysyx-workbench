#include "PerfCounter.hpp"
#include "spdlog/spdlog.h"
#include <tabulate/table.hpp>

using namespace _PerfCtrImp;
using namespace tabulate;

static void _SetTableFmt(Table &t) {
  assert(t.size() >= 2);
  t.format().font_align(FontAlign::right);
  t.row(0).format().font_align(FontAlign::center);

  for (auto &row : t) {
    row.format().hide_border_top();
  }

  t[0].format().show_border_top();
  t[1].format().show_border_top();
}
void _PrintTable(Table &t, std::ostream &os) {
  _SetTableFmt(t);
  os << t << std::endl;
}

void EXUPerfCounter::_dump(size_t *instCnts, size_t *cycCnts, size_t num,
                           const char *(*nameFunc)(int), std::ostream &os) {
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
  _PrintTable(t, os);
}

const char *IFUStateCounter::nameOfState(int s) {
  const char *names[] = {
      "IDLE",
      "WAIT_INST",
      "WAIT_LATER",
  };
  return names[s];
}
void IFUStateCounter::dumpStatistics(std::ostream &os) {
  os << "IFU State Counter Statistics:\n";
  os << "total instruction fetch count: " << totalFetchCount << "\n";
  os << "state statistics:\n";
  // spdlog::info("IFU State Counter Statistics:");
  // fmt::println("total instruction fetch count: {}", totalFetchCount);
  // fmt::println("state statistics:");
  Table t;
  t.add_row({"State", "Count", "Percent", "Count\n[exclu fetch]",
             "Percent\n[exclu fetch]"});

  auto totCycles = sim_get_cycle();
  for (size_t i = 0; i < STATE_NUM; i++) {
    double perc = totCycles == 0
                      ? NAN
                      : ((double)countOfState[i] / (double)totCycles) * 100.0;
    double percNoFetch =
        totCycles == 0
            ? NAN
            : ((double)countOfStateWhenNoFetch[i] / (double)totCycles) * 100.0;

    t.add_row(RowStream{} << nameOfState(i) << countOfState[i] << perc
                          << countOfStateWhenNoFetch[i] << percNoFetch);
  }
  _PrintTable(t, os);
}

// void AXI4CounterBase::dumpStatisticsTitle(){
// 	fmt::println("  {:18} : {:>8} {:>10} {:>8} {:>8}",
// 							 "name", "txns",
// "cycles", "avg_lat", "max_lat");
// }
void AXI4CounterBase::dumpStatistics(std::ostream &os) {
  spdlog::error("AXI4CounterBase::dumpStatistics unimpled!!!");
  // // name : total_txns total_cycles avg_latency max_latency max_time_beg
  // // max_time_end
  // double avg_latency = transaction_count == 0 ? NAN
  //                                             : (double)total_latency_cycles
  //                                             /
  //                                                   (double)transaction_count;
  // fmt::println("  {:18} : {:>8} {:>10} {:>8.2f} {:>8} (at sim time {} to
  // {})", ctrName,
  //              transaction_count, total_latency_cycles, avg_latency,
  //              maxRecord.cycles, maxRecord.startTime, maxRecord.endTime);
}

void AXI4PerfCounterManager::dumpStatistics(std::ostream &os) {
  os << "AXI4 Performance Counters Statistics:\n";
  // os << "AXI4 Read Counters:\n";

  Table t;
  t.add_row({"Name", "Transactions", "Total\nCycles", "Avg\nLatency",
             "Max\nLatency", "Max Start\n(sim time)" });
  for (auto &ctr : rdCounters) {
    double avg_latency =
        ctr.transaction_count == 0
            ? NAN
            : (double)ctr.total_latency_cycles / (double)ctr.transaction_count;
    t.add_row(RowStream{} << ctr.ctrName << ctr.transaction_count
                          << ctr.total_latency_cycles << avg_latency
                          << ctr.maxRecord.cycles << ctr.maxRecord.startTime);
  }
  _PrintTable(t, os);

  // os << "AXI4 Write Counters:\n";
}
