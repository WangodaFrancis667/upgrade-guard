#include "upgrade_guard/modules/RuleFactories.hpp"

namespace upgrade_guard::modules {
namespace {

class RebootRule final : public ports::IReadinessRule {
public:
  [[nodiscard]] std::string id() const override { return "UG-RBT-001"; }
  [[nodiscard]] domain::Finding evaluate(const domain::SystemSnapshot &s) const override {
    domain::Finding f{"UG-RBT-001",
                      domain::CheckStatus::passed,
                      domain::Severity::info,
                      "Pending reboot",
                      "No pending reboot marker was detected.",
                      {{"reboot required", s.reboot.reboot_required ? "true" : "false"}},
                      "No action is required for this check.",
                      domain::Confidence::high,
                      true,
                      "docs/supported-platforms.md"};
    for (const auto &package : s.reboot.packages) {
      f.evidence.push_back({"reboot package", package});
    }
    if (s.reboot.read_error) {
      f.status = domain::CheckStatus::unknown;
      f.severity = domain::Severity::warning;
      f.explanation = "Pending reboot state could not be read.";
      f.recommendation = "Inspect /var/run/reboot-required manually.";
      f.collector_complete = false;
    } else if (s.reboot.reboot_required) {
      f.status = domain::CheckStatus::warning;
      f.severity = domain::Severity::warning;
      f.explanation = "The system already has a pending reboot marker.";
      f.recommendation = "Reboot into a settled state before attempting a release upgrade.";
    }
    return f;
  }
};

} // namespace

std::vector<std::unique_ptr<ports::IReadinessRule>> make_reboot_rules() {
  std::vector<std::unique_ptr<ports::IReadinessRule>> rules;
  rules.push_back(std::make_unique<RebootRule>());
  return rules;
}

} // namespace upgrade_guard::modules
