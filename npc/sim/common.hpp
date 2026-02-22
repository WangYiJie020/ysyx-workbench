#pragma once
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <format>

#include "vsrc.hpp"

inline std::string getOutputDir(std::string_view prefix) {
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
inline auto newFileLoggerSink(const std::string &name) {
  auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
      std::format("{}/{}.log", getOutputDir("build/logs"), name), true);
  file_sink->set_level(spdlog::level::debug);
  return file_sink;
}
