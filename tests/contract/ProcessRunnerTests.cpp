#include "upgrade_guard/ports/ProcessRunnerFactory.hpp"

#include "../TestSupport.hpp"

namespace upgrade_guard::tests {

void run_process_runner_tests() {
  const auto runner = make_posix_process_runner();
  auto ok = runner->run({"true", {}, std::chrono::milliseconds(1000), 1024, {}});
  require(ok.ok() && ok.value().exit_code == 0, "process runner executes allowed fixed command");
  auto missing = runner->run({"not-upgrade-guard-allowed", {}, std::chrono::milliseconds(100), 1024, {}});
  require(missing.ok() && missing.value().spawn_failed, "process runner rejects unavailable executable");

  auto nonzero = runner->run({"dpkg", {"--upgrade-guard-invalid-option"}, std::chrono::milliseconds(1000), 1024, {}});
  require(nonzero.ok() && nonzero.value().exit_code != 0, "process runner preserves nonzero exit status");
  require(nonzero.ok() && !nonzero.value().stderr_text.empty(), "process runner captures stderr separately");

  auto stdout_limit = runner->run({"dpkg", {"--version"}, std::chrono::milliseconds(1000), 1, {}});
  require(stdout_limit.ok() && stdout_limit.value().stdout_truncated && stdout_limit.value().stdout_text.size() == 1,
          "process runner bounds stdout");

  auto stderr_limit = runner->run({"dpkg", {"--upgrade-guard-invalid-option"}, std::chrono::milliseconds(1000), 1, {}});
  require(stderr_limit.ok() && stderr_limit.value().stderr_truncated && stderr_limit.value().stderr_text.size() == 1,
          "process runner bounds stderr");

  auto timeout = runner->run({"apt-get", {"--version"}, std::chrono::milliseconds(0), 1024, {}});
  require(timeout.ok() && timeout.value().timed_out, "process runner terminates and reaps timed-out children");
  require(timeout.ok() && timeout.value().signaled && timeout.value().termination_signal != 0,
          "process runner reports timeout signal termination");
}

} // namespace upgrade_guard::tests

int main() {
  upgrade_guard::tests::run_process_runner_tests();
  return upgrade_guard::tests::done();
}
