#pragma once

#include "upgrade_guard/ports/IProcessRunner.hpp"

#include <map>

namespace upgrade_guard::tests {

class FakeProcessRunner final : public ports::IProcessRunner {
public:
  void add(std::string executable, ports::ProcessResult result) { results_[std::move(executable)] = std::move(result); }

  [[nodiscard]] domain::Result<ports::ProcessResult> run(const ports::ProcessRequest &request) const override {
    const auto found = results_.find(request.executable);
    if (found == results_.end()) {
      ports::ProcessResult result;
      result.spawn_failed = true;
      result.failure_message = "missing fake";
      return result;
    }
    auto result = found->second;
    if (request.max_output_bytes < result.stdout_text.size()) {
      result.stdout_text.resize(request.max_output_bytes);
      result.truncated = true;
    }
    return result;
  }

private:
  std::map<std::string, ports::ProcessResult> results_;
};

} // namespace upgrade_guard::tests
