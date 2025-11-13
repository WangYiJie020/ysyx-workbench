#pragma once
#include <functional>
#include <string_view>
#include <ranges>
#include <vector>
#include <charconv>
#include <tuple>

namespace clscmd {
	using std::string_view;
	using std::function;

inline static auto make_toks(string_view str){
	using namespace std::views;
	return str
		| split(' ')
		| transform([](auto&& rng) {
				return string_view(rng.begin(), rng.size());
		});
}

using rawtoks_view_t = decltype(make_toks("")|std::views::drop(0));
using toks_t = std::vector<string_view>;

struct command_t{
	string_view name;
	string_view description;
	function<void(toks_t)> invoke;
};

inline void _parse(string_view s,int& v){
	std::from_chars(s.begin(),s.end(),v);
}

template <typename Class, typename Ret, typename... Args>
command_t make_command(string_view name,string_view description, Class* obj, Ret(Class::*func)(Args...)) {
	using namespace std;
    return command_t{
        .name=name,
		.description=description,
		.invoke=[obj, func](toks_t toks) {
            if (toks.size() != sizeof...(Args)) {
				// caller shold ensure toks num
                throw runtime_error("argument count mismatch");
            }

            tuple<decay_t<Args>...> args;
            auto fill_args = [&]<size_t...Is>(index_sequence<Is...>) {
                ((_parse(toks[Is], get<Is>(args))), ...);
            };
            fill_args(index_sequence_for<Args...>{});

            apply([&](auto&&... unpacked) {
                (obj->*func)(unpacked...);
            }, args);
        }
    };
}
}
