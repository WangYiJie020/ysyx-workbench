#ifndef __KASAN_INIT_H__
#define __KASAN_INIT_H__

void initialize_heap();
void initialize_kasan();
void call_global_ctors();

inline void init_kasan() {
  // Needed to invoke KASan globals instrumentation.
  initialize_heap();
  initialize_kasan();
  call_global_ctors();
}

#endif // __KASAN_INIT_H__
