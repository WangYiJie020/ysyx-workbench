#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string_view>
#include <vector>

#include "VNetlistTop.h"
#include "verilated.h"

namespace {
constexpr uint32_t kPmemBase = 0x80000000u;
constexpr size_t kPmemSize = 128u * 1024u * 1024u;
constexpr uint32_t kUartAddr = 0x10000000u;
std::vector<uint8_t> pmem(kPmemSize);

uint32_t readWord(uint32_t addr) {
  if (addr >= kPmemBase && addr - kPmemBase <= kPmemSize - 4) {
    const uint32_t offset = (addr - kPmemBase) & ~uint32_t{3};
    return uint32_t{pmem[offset]} | (uint32_t{pmem[offset + 1]} << 8) |
           (uint32_t{pmem[offset + 2]} << 16) |
           (uint32_t{pmem[offset + 3]} << 24);
  }
  std::fprintf(stderr, "WARNING: unmapped netlist read at 0x%08" PRIx32 "\n", addr);
  return 0;
}

void loadImage(const char *path) {
  FILE *file = std::fopen(path, "rb");
  if (file == nullptr) {
    std::perror(path);
    std::exit(EXIT_FAILURE);
  }
  std::fseek(file, 0, SEEK_END);
  const long size = std::ftell(file);
  std::rewind(file);
  if (size < 0 || static_cast<unsigned long>(size) > kPmemSize ||
      std::fread(pmem.data(), 1, static_cast<size_t>(size), file) !=
          static_cast<size_t>(size)) {
    std::fprintf(stderr, "cannot load image '%s'\n", path);
    std::fclose(file);
    std::exit(EXIT_FAILURE);
  }
  std::fclose(file);
  std::printf("Loaded %ld bytes from %s at 0x%08" PRIx32 "\n", size, path, kPmemBase);
}
} // namespace

extern "C" int pmem_read(int addr) { return static_cast<int>(readWord(addr)); }

extern "C" void pmem_write(int addrValue, int dataValue, signed char maskValue) {
  const uint32_t addr = static_cast<uint32_t>(addrValue);
  const uint32_t data = static_cast<uint32_t>(dataValue);
  const uint8_t mask = static_cast<uint8_t>(maskValue);
  if (addr == kUartAddr) {
    std::putchar(static_cast<int>(data & 0xffu));
    std::fflush(stdout);
    return;
  }
  if (addr < kPmemBase || addr - kPmemBase > kPmemSize - 4) {
    std::fprintf(stderr, "unmapped netlist write at 0x%08" PRIx32 "\n", addr);
    std::exit(EXIT_FAILURE);
  }
  const uint32_t offset = (addr - kPmemBase) & ~uint32_t{3};
  for (unsigned lane = 0; lane < 4; ++lane) {
    if ((mask & (1u << lane)) != 0) pmem[offset + lane] = data >> (lane * 8);
  }
}

int main(int argc, char **argv) {
  const char *image = nullptr;
  uint64_t maxCycles = 1000000000ull;
  for (int i = 1; i < argc; ++i) {
    const std::string_view arg(argv[i]);
    if (arg.starts_with("+image=")) image = argv[i] + std::strlen("+image=");
    if (arg.starts_with("+max-cycles="))
      maxCycles = std::strtoull(argv[i] + std::strlen("+max-cycles="), nullptr, 10);
  }
  if (image == nullptr) {
    std::fprintf(stderr, "missing +image=<binary>\n");
    return EXIT_FAILURE;
  }
  loadImage(image);

  auto context = std::make_unique<VerilatedContext>();
  context->commandArgs(argc, argv);
  auto top = std::make_unique<VNetlistTop>(context.get());
  top->clock = 0;
  top->reset = 1;
  top->eval();

  for (uint64_t cycle = 0; cycle < maxCycles; ++cycle) {
    top->reset = cycle < 5;
    top->clock = 1;
    top->eval();
    context->timeInc(1);
    if (top->halted) {
      std::printf("NPC netlist halted after %" PRIu64 " cycles\n", cycle);
      top->final();
      return EXIT_SUCCESS;
    }
    top->clock = 0;
    top->eval();
    context->timeInc(1);
  }

  std::fprintf(stderr, "netlist timeout after %" PRIu64 " cycles\n", maxCycles);
  top->final();
  return EXIT_FAILURE;
}
