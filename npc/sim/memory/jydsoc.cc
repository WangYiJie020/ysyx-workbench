#include "mem.hpp"
#include <cstdint>

// static std::shared_ptr<direct_mapped_mem> sdram_ptr;
static std::shared_ptr<direct_mapped_mem> irom_ptr = nullptr;
