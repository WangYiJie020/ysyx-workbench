#include "common.hpp"

std::string getOutputDir(std::string_view prefix) {
  // get timestamp once
  // later user use the same timestamp
  static auto now = std::chrono::system_clock::now();
  std::string_view git_commit_hash = _STR(GIT_COMMIT_HASH);
  auto shortGitHash = git_commit_hash.substr(0, 8);
  std::string logDir =
      std::format("{}/{}/{:%m%dT%H_%M_%S}", prefix, shortGitHash, now);
  system(("mkdir -p " + logDir).c_str());
  return logDir;
}


bool isCIEnv() {
  const char *ci_env = std::getenv("GITHUB_ACTIONS");
  return ci_env != nullptr && std::string_view(ci_env) == "true";
}

std::string runCommand(const std::string &cmd) {
  spdlog::debug("Running command: {}", cmd);
  std::array<char, 2048> buffer{};
  std::string result;

  FILE *pipe = popen(cmd.c_str(), "r");
  assert(pipe && "popen failed");

  while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
    result += buffer.data();
  }

  int ret = pclose(pipe);
  assert(ret != -1 && "pclose failed");

  return result;
}

uint32_t _readElfSymValue(const std::string &elf,
                                 const std::string &sym) {
  std::string output = runCommand(
      std::format("readelf -s {} | awk '/{}/{{print $2}}'", elf, sym));
  uint32_t value = 0;
  try {
    value = std::stoul(output, nullptr, 16);
  } catch (const std::exception &e) {
    spdlog::error(
        "Failed to parse symbol '{}' value from ELF '{}': {}, output was '{}'",
        sym, elf, e.what(), output);
  }
  return value;
}
