#include "upgrade_guard/modules/RuleFactories.hpp"

#include <sstream>

namespace upgrade_guard::modules {
namespace {

class DkmsCollector final : public ports::ICollector {
public:
  explicit DkmsCollector(const ports::IProcessRunner &runner) : runner_(runner) {}
  [[nodiscard]] std::string name() const override { return "dkms"; }
  [[nodiscard]] domain::Result<void> collect(domain::SystemSnapshot &snapshot) const override {
    auto result = runner_.run({"dkms", {"status"}, std::chrono::milliseconds(4000), 65536, {}});
    if (!result.ok() || (result.ok() && result.value().spawn_failed)) {
      snapshot.dkms.command_missing = true;
      return {};
    }
    snapshot.dkms.installed = true;
    if (result.value().timed_out || result.value().exit_code != 0 || result.value().truncated) {
      domain::add_issue(snapshot, name(), "dkms status did not complete successfully");
    }
    std::istringstream in(result.value().stdout_text);
    std::string line;
    while (std::getline(in, line)) {
      if (line.empty()) {
        continue;
      }
      snapshot.dkms.modules.push_back(line);
      if (line.find("bad") != std::string::npos || line.find("failed") != std::string::npos ||
          line.find("broken") != std::string::npos) {
        snapshot.dkms.failed_modules.push_back(line);
      }
      if (line.find(": added") != std::string::npos || line.find(": built") != std::string::npos) {
        snapshot.dkms.incomplete_modules.push_back(line);
      }
      if (line.find(',') == std::string::npos || line.find(':') == std::string::npos) {
        snapshot.dkms.parse_errors.push_back(line);
      }
    }
    return {};
  }

private:
  const ports::IProcessRunner &runner_;
};

} // namespace

std::unique_ptr<ports::ICollector> make_dkms_collector(const ports::IProcessRunner &runner) {
  return std::make_unique<DkmsCollector>(runner);
}

} // namespace upgrade_guard::modules
