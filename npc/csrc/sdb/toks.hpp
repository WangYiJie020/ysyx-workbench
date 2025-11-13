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

	using toks_t = std::vector<string_view>;

namespace _impl {
template <typename T>
concept CanFromChars = std::integral<T> || std::floating_point<T>;

inline auto make_toks(string_view str){
	using namespace std::views;
	return str | split(' ')
		| transform([](auto&& rng) {
				return string_view(rng.begin(), rng.size());
		});
}
using rawtoks_view_t = decltype(make_toks("")|std::views::drop(0));

auto parse(string_view s,_impl::CanFromChars auto& v){
	return std::from_chars(s.begin(),s.end(),v).ec;
}
inline auto parse(string_view s,string_view& v){
	v=s;
	return std::errc();
}

}

using invoke_error = std::errc;
constexpr invoke_error invoke_success=invoke_error();

struct command_t{
	string_view description;
	function<invoke_error(toks_t)> invoke;
	command_t():description(),invoke(){
	}

	template <typename Class, typename Ret, typename... Args,typename... Defaults>
	command_t(
		string_view _description,
		Class* obj, Ret(Class::*func)(Args...),
		std::tuple<Defaults...> defaults = {}
	){
		using namespace std;
		description=_description,
		invoke=[obj, func, defaults](toks_t toks) {
			bool ok;
			errc ec;
            tuple<decay_t<Args>...> args;
			constexpr size_t N_args = sizeof...(Args);
            constexpr size_t N_def  = sizeof...(Defaults);

			const size_t n_tok = toks.size();

            if (n_tok > N_args) {
                throw runtime_error("too many arguments");
            }
            if (n_tok + N_def < N_args) {
                throw runtime_error("not enough arguments even with defaults");
            }

            constexpr size_t I_tokend = N_args - N_def;

            [&]<size_t... Is>(index_sequence<Is...>) {
			   (([&](){
				 if constexpr (Is >= I_tokend) {
				 	constexpr size_t def_idx = Is - I_tokend;
				 	get<Is>(args) = get<def_idx>(defaults);
				 } else {
				 	if (Is < n_tok) {
						if(!ok)return;
						ec = _impl::parse(toks[Is], get<Is>(args));
						ok=(ec==errc());
					}
				 	throw runtime_error("missing required argument");
				 }
			   }()),...);
           	}(index_sequence_for<Args...>{});

			if(!ok)return invoke_error(ec);

            std::apply([&](auto&&... unpacked){
                (obj->*func)(unpacked...);
            }, args);
			return invoke_success;
        };
	}

};
using command_table=std::unordered_map<string_view, command_t>;

}
