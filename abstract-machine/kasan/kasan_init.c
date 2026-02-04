#include "kasan_init.h"
#include "klib.h"
void kasan_init() {
  void initialize_heap();
  void initialize_kasan();
  void call_global_ctors();
  _no_asan_kmemzero((void *)KASAN_SHADOW_MEMORY_START,
                    KASAN_SHADOW_MEMORY_SIZE);

  // Needed to invoke KASan globals instrumentation.
  initialize_heap();
  initialize_kasan();
  call_global_ctors();
}
