#include "upgrade_guard/ports/ProcessRunnerFactory.hpp"

#include "../TestSupport.hpp"

namespace upgrade_guard::tests {

void run_process_runner_tests() {
  const auto runner = make_posix_process_runner();
  auto ok = runner->run({"true", {}, std::chrono::milliseconds(1000), 1024, {}});
  require(ok.ok() && ok.value().exit_code == 0, "process runner executes allowed fixed command");
  auto missing = runner->run({"not-upgrade-guard-allowed", {}, std::chrono::milliseconds(100), 1024, {}});
  require(missing.ok() && missing.value().spawn_failed, "process runner rejects unavailable executable");
}

} // namespace upgrade_guard::tests
