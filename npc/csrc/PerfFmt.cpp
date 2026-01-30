#include "PerfCounter.hpp"
#include <tabulate/table.hpp>

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
		t.row(i+1).format().hide_border_bottom();
		t.row(i+1).format().hide_border_top();
  }

	t[1].format().show_border_top();
	t[num].format().show_border_bottom();

	t.format().font_align(FontAlign::right);

	t.column(0).format().font_align(FontAlign::center);

	t.row(0).format().font_align(FontAlign::center);

	std::cout << t << std::endl;
}
