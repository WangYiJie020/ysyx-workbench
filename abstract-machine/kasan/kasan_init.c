#include "kasan_init.h"
void kasan_init() {
  void initialize_heap();
  void initialize_kasan();
  void call_global_ctors();
  _no_asan_kmemset((void *)KASAN_SHADOW_MEMORY_START, KASAN_SHADOW_MEMORY_SIZE,
                   0);

  // Needed to invoke KASan globals instrumentation.
  initialize_heap();
  initialize_kasan();
  call_global_ctors();
}
