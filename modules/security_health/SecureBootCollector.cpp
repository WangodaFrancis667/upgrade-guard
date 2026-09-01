#include "upgrade_guard/modules/RuleFactories.hpp"

namespace upgrade_guard::modules {
namespace {

class SecureBootCollector final : public ports::ICollector {
public:
  explicit SecureBootCollector(const ports::IProcessRunner &runner) : runner_(runner) {}
  [[nodiscard]] std::string name() const override { return "secure-boot"; }
  [[nodiscard]] domain::Result<void> collect(domain::SystemSnapshot &snapshot) const override {
    auto result = runner_.run({"mokutil", {"--sb-state"}, std::chrono::milliseconds(3000), 8192, {}});
    if (!result.ok() || result.value().spawn_failed) {
      snapshot.secure_boot.mokutil_installed = false;
      snapshot.secure_boot.state_unknown = true;
      return {};
    }
    snapshot.secure_boot.mokutil_installed = true;
    snapshot.secure_boot.raw_state = result.value().stdout_text;
    snapshot.secure_boot.enabled = result.value().stdout_text.find("enabled") != std::string::npos;
    snapshot.secure_boot.state_unknown = result.value().stdout_text.empty() || result.value().exit_code != 0 ||
                                         result.value().timed_out || result.value().truncated;
    return {};
  }

private:
  const ports::IProcessRunner &runner_;
};

} // namespace

std::unique_ptr<ports::ICollector> make_secure_boot_collector(const ports::IProcessRunner &runner) {
  return std::make_unique<SecureBootCollector>(runner);
}

} // namespace upgrade_guard::modules
