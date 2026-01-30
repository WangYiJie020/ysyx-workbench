#include "PerfCounter.hpp"
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
void _PrintTable(Table &t){
	_SetTableFmt(t);
	std::cout <<std::setprecision(3)<< t << std::endl;
}

void EXUPerfCounter::_dump(size_t *instCnts, size_t *cycCnts, size_t num,
                           const char *(*nameFunc)(int)) {
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
	_PrintTable(t);
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
  fmt::println("total instruction fetch count: {}", totalFetchCount);
  fmt::println("state statistics:");
  // fmt::println("    {:10} : {:<18} {:<18}", "state", "count",
  //              "count[exclu fetch]");
  // auto totCycles = sim_get_cycle();
  // for (size_t i = 0; i < STATE_NUM; i++) {
  //   double perc = totCycles == 0
  //                     ? NAN
  //                     : ((double)countOfState[i] / (double)totCycles) *
  //                     100.0;
  //   double percNoFetch =
  //       totCycles == 0
  //           ? NAN
  //           : ((double)countOfStateWhenNoFetch[i] / (double)totCycles) *
  //           100.0;
  //
  //   fmt::println("    {:10} : {:>8} ({:6.3f}%) {:>8} ({:6.3f}%)",
  //                _name_of_ifu_state((State)i), countOfState[i], perc,
  //                countOfStateWhenNoFetch[i], percNoFetch);
  // }
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

    t.add_row(RowStream{} << _name_of_ifu_state((State)i) << countOfState[i]
                          << perc << countOfStateWhenNoFetch[i] << percNoFetch);
  }
  _PrintTable(t);
}
