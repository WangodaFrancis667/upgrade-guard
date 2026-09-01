#pragma once

#include "upgrade_guard/domain/Result.hpp"

#include <chrono>
#include <map>
#include <string>
#include <vector>

namespace upgrade_guard::ports {

struct ProcessRequest {
  std::string executable;
  std::vector<std::string> arguments;
  std::chrono::milliseconds timeout{3000};
  std::size_t max_output_bytes{65536};
  std::map<std::string, std::string> environment;
};

struct ProcessResult {
  int exit_code{-1};
  std::string stdout_text;
  std::string stderr_text;
  bool timed_out{false};
  bool truncated{false};
  bool spawn_failed{false};
  std::string failure_message;
};

class IProcessRunner {
public:
  virtual ~IProcessRunner() = default;
  [[nodiscard]] virtual domain::Result<ProcessResult> run(const ProcessRequest &request) const = 0;
};

} // namespace upgrade_guard::ports
