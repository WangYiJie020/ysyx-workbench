#pragma once
#include <functional>
#include <string_view>
#include <ranges>
#include <vector>
#include <charconv>
#include <tuple>
#include <concepts>

namespace clscmd {
	using std::string_view;
	using std::function;

inline static auto make_toks(string_view str){
	using namespace std::views;
	return str | split(' ')
		| transform([](auto&& rng) {
				return string_view(rng.begin(), rng.size());
		});
}

using rawtoks_view_t = decltype(make_toks("")|std::views::drop(0));
using toks_t = std::vector<string_view>;
using invoke_error = std::errc;
constexpr invoke_error invoke_success=invoke_error();

struct command_t{
	string_view name;
	string_view description;
	function<invoke_error(toks_t)> invoke;
};

namespace _impl {
template <typename T>
concept CanFromChars = std::integral<T> || std::floating_point<T>;
}

auto parse(string_view s,_impl::CanFromChars auto& v){
	return std::from_chars(s.begin(),s.end(),v).ec;
}
inline auto parse(string_view s,string_view& v){
	v=s;
	return std::errc();
}

template <typename Class, typename Ret, typename... Args>
command_t make_command(
	string_view name,
	string_view description,
   	Class* obj, Ret(Class::*func)(Args...)
) {
	using namespace std;
    return command_t{
        .name=name,
		.description=description,
		.invoke=[obj, func](toks_t toks) {
            if (toks.size() != sizeof...(Args)) {
				// caller shold ensure toks num
                throw runtime_error("argument count mismatch");
            }
			bool ok;
			errc ec;
            tuple<decay_t<Args>...> args;
            [&]<size_t...Is>(index_sequence<Is...>) {
                (([&] {
                    if (!ok) return;
                    ec = parse(toks[Is], std::get<Is>(args));
					ok = (ec==errc());
                }()), ...);
            }(index_sequence_for<Args...>{});
			if(!ok)return ec;

            apply([&](auto&&... unpacked) {
                (obj->*func)(unpacked...);
            }, args);
			return invoke_error();
        }
    };
}
}
