#include "upgrade_guard/modules/RuleFactories.hpp"

#include <sstream>

namespace upgrade_guard::modules {
namespace {

std::vector<std::string> lines(const std::string &text) {
  std::istringstream in(text);
  std::vector<std::string> result;
  std::string line;
  while (std::getline(in, line)) {
    if (!line.empty()) {
      result.push_back(line);
    }
  }
  return result;
}

class DpkgCollector final : public ports::ICollector {
public:
  explicit DpkgCollector(const ports::IProcessRunner &runner) : runner_(runner) {}
  [[nodiscard]] std::string name() const override { return "dpkg"; }
  [[nodiscard]] domain::Result<void> collect(domain::SystemSnapshot &snapshot) const override {
    auto audit = runner_.run({"dpkg", {"--audit"}, std::chrono::milliseconds(3000), 65536, {}});
    if (audit.ok() && !audit.value().spawn_failed && !audit.value().timed_out && audit.value().exit_code == 0) {
      snapshot.packages.dpkg_audit = lines(audit.value().stdout_text);
    } else {
      domain::add_issue(snapshot, name(), audit.ok() ? audit.value().failure_message : audit.error().message);
    }

    auto holds = runner_.run({"apt-mark", {"showhold"}, std::chrono::milliseconds(3000), 65536, {}});
    if (holds.ok() && !holds.value().spawn_failed && !holds.value().timed_out && holds.value().exit_code == 0) {
      snapshot.packages.held_packages = lines(holds.value().stdout_text);
    } else {
      domain::add_issue(snapshot, name(), holds.ok() ? holds.value().failure_message : holds.error().message);
    }
    return {};
  }

private:
  const ports::IProcessRunner &runner_;
};

} // namespace

std::unique_ptr<ports::ICollector> make_dpkg_collector(const ports::IProcessRunner &runner) {
  return std::make_unique<DpkgCollector>(runner);
}

} // namespace upgrade_guard::modules
