#include "upgrade_guard/modules/RuleFactories.hpp"

namespace upgrade_guard::modules {
namespace {

class SecureBootRule final : public ports::IReadinessRule {
public:
  [[nodiscard]] std::string id() const override { return "UG-SEC-001"; }
  [[nodiscard]] domain::Finding evaluate(const domain::SystemSnapshot &s) const override {
    domain::Finding f{"UG-SEC-001",
                      domain::CheckStatus::passed,
                      domain::Severity::info,
                      "Secure Boot and external module state",
                      "Secure Boot and DKMS evidence did not reveal a blocking combination.",
                      {{"mokutil installed", s.secure_boot.mokutil_installed ? "true" : "false"},
                       {"secure boot enabled", s.secure_boot.enabled ? "true" : "false"},
                       {"dkms modules", std::to_string(s.dkms.modules.size())}},
                      "No action is required for this check.",
                      domain::Confidence::medium,
                      true,
                      "docs/threat-model.md"};
    if (s.platform.has_value() && s.platform->container_detected) {
      f.status = domain::CheckStatus::unknown;
      f.severity = domain::Severity::warning;
      f.explanation = "Secure Boot evidence is container-limited.";
      f.recommendation = "Validate Secure Boot and module signing in a native system or VM.";
      f.collector_complete = false;
    } else if (s.secure_boot.state_unknown) {
      f.status = domain::CheckStatus::unknown;
      f.severity = domain::Severity::warning;
      f.explanation = "Secure Boot state could not be determined read-only.";
      f.recommendation = "Inspect Secure Boot state before upgrading systems with external modules.";
      f.collector_complete = false;
    } else if (s.secure_boot.enabled && !s.dkms.modules.empty()) {
      f.status = domain::CheckStatus::warning;
      f.severity = domain::Severity::warning;
      f.explanation = "Secure Boot is enabled while DKMS-managed external modules are present.";
      f.recommendation = "Confirm module signing and MOK enrollment state before upgrading.";
    }
    return f;
  }
};

} // namespace

std::vector<std::unique_ptr<ports::IReadinessRule>> make_security_rules() {
  std::vector<std::unique_ptr<ports::IReadinessRule>> rules;
  rules.push_back(std::make_unique<SecureBootRule>());
  return rules;
}

} // namespace upgrade_guard::modules
