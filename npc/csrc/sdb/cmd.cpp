#include "cmd.hpp"
using namespace clscmd;
invoke_error clscmd::exec(const command_table& table,string_view cmdline){
	auto toks=make_rawtoks(cmdline);
	if(toks.empty())return invoke_success;
	auto it=table.find(toks.front());
	if(it==table.end())return std::errc::no_such_process;
	auto cmd=it->second;
	auto args=toks|std::views::drop(1);
	return cmd.invoke(toks_t(args.begin(),args.end()));
}
