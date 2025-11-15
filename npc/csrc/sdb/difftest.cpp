#include "sdb.hpp"
#include <dlfcn.h>

using namespace std;
using namespace sdb;

#define COND_ENABLE if constexpr(_ENABLE_DIFFTEST)

typedef void(*ref_difftest_init_t)(int port);
typedef void(*ref_difftest_memcpy_t)(paddr_t addr, void* buf, size_t n, bool direction);
typedef void(*ref_difftest_regcpy_t)(void* regs, bool direction);
typedef void(*ref_difftest_exec_t)(size_t n);

enum { DIFFTEST_TO_DUT, DIFFTEST_TO_REF };

struct sdb::_impl::difftest_imp{
	void* handle=nullptr;
	ref_difftest_init_t ref_init=nullptr;
	ref_difftest_memcpy_t ref_memcpy=nullptr;
	ref_difftest_regcpy_t ref_regcpy=nullptr;
	ref_difftest_exec_t ref_exec=nullptr;

	void _meta_load(auto fn, const char* name){
		fn=(decltype(fn))dlsym(handle, name);
		if(!fn)
			throw runtime_error(format("load {} failed: {}", name, dlerror()));
	}


	~difftest_imp(){
		if(handle){
			dlclose(handle);
			handle=nullptr;
		}
	}
	void load(string_view so_file){
		handle=dlopen(so_file.data(), RTLD_LAZY);
		if(!handle){
			throw runtime_error(format("difftest dlopen {} failed: {}", so_file, dlerror()));
		}
		_meta_load(ref_init, "difftest_init");
		_meta_load(ref_memcpy, "difftest_memcpy");
		_meta_load(ref_regcpy, "difftest_regcpy");
		_meta_load(ref_exec, "difftest_exec");
	}

};

void _impl::_deleter_difftest::operator()(difftest_imp* ptr){
	if(ptr){delete ptr;}
}

void debuger::load_difftest_ref(string_view so_file,size_t img_size){COND_ENABLE{
	_imp_difftest=_impl::difftest_imptr(new _impl::difftest_imp());
	auto& imp=*_imp_difftest;
	printf("Difftest: Loading ref so from %.*s\n",
		(int)so_file.size(),so_file.data());
	imp.load(so_file);
	imp.ref_init(0);
	
	
}}

void debuger::_difftest_step(paddr_t pc,paddr_t npc){COND_ENABLE{
}}
