#include "upgrade_guard/modules/RuleFactories.hpp"

#include <functional>

namespace upgrade_guard::modules {
namespace {

using Eval = std::function<domain::Finding(const domain::SystemSnapshot &)>;

class Rule final : public ports::IReadinessRule {
public:
  Rule(std::string id, Eval eval) : id_(std::move(id)), eval_(std::move(eval)) {}
  [[nodiscard]] std::string id() const override { return id_; }
  [[nodiscard]] domain::Finding evaluate(const domain::SystemSnapshot &snapshot) const override { return eval_(snapshot); }

private:
  std::string id_;
  Eval eval_;
};

domain::Finding fallback(const domain::SystemSnapshot &s) {
  domain::Finding f{"UG-KRN-001",
                    domain::CheckStatus::passed,
                    domain::Severity::info,
                    "Fallback kernel availability",
                    "An installed fallback kernel was detected.",
                    {{"installed kernels", std::to_string(s.kernel.installed_kernels.size())}},
                    "No action is required for this check.",
                    domain::Confidence::medium,
                    true,
                    "docs/supported-platforms.md"};
  if (s.platform.has_value() && s.platform->container_detected) {
    f.status = domain::CheckStatus::unknown;
    f.severity = domain::Severity::warning;
    f.explanation = "Kernel fallback evidence is container-limited because containers share the host kernel.";
    f.recommendation = "Validate kernel fallback availability in a native system or VM.";
    f.collector_complete = false;
  } else if (s.kernel.installed_kernels.empty()) {
    f.status = domain::CheckStatus::unknown;
    f.severity = domain::Severity::warning;
    f.explanation = "Installed kernel package evidence was unavailable.";
    f.recommendation = "Inspect dpkg-query availability and kernel package state.";
    f.collector_complete = false;
  } else if (!s.kernel.has_fallback_kernel) {
    f.status = domain::CheckStatus::warning;
    f.severity = domain::Severity::warning;
    f.explanation = "Only one installed kernel package was detected.";
    f.recommendation = "Confirm a known-good fallback kernel is available before upgrading.";
  }
  return f;
}

domain::Finding consistency(const domain::SystemSnapshot &s) {
  domain::Finding f{"UG-KRN-002",
                    domain::CheckStatus::passed,
                    domain::Severity::info,
                    "Kernel, headers and initramfs consistency",
                    "No kernel/header/initramfs inconsistency was detected.",
                    {},
                    "No action is required for this check.",
                    domain::Confidence::medium,
                    true,
                    "docs/supported-platforms.md"};
  if (s.platform.has_value() && s.platform->container_detected) {
    f.status = domain::CheckStatus::unknown;
    f.severity = domain::Severity::warning;
    f.explanation = "Kernel support-file evidence is container-limited.";
    f.recommendation = "Validate headers and initramfs in a native system or VM.";
    f.collector_complete = false;
    return f;
  }
  for (const auto &issue : s.kernel.missing_headers) {
    f.evidence.push_back({"missing header", issue});
  }
  for (const auto &issue : s.kernel.initramfs_issues) {
    f.evidence.push_back({"initramfs issue", issue});
  }
  if (!f.evidence.empty()) {
    f.status = domain::CheckStatus::warning;
    f.severity = domain::Severity::warning;
    f.explanation = "Kernel support files appear incomplete or suspicious.";
    f.recommendation = "Inspect kernel headers and initramfs files manually.";
  }
  return f;
}

domain::Finding dkms(const domain::SystemSnapshot &s) {
  domain::Finding f{"UG-DKM-001",
                    domain::CheckStatus::passed,
                    domain::Severity::info,
                    "DKMS module health",
                    "DKMS is absent or no failed DKMS modules were reported.",
                    {{"dkms installed", s.dkms.installed ? "true" : "false"},
                     {"dkms command missing", s.dkms.command_missing ? "true" : "false"}},
                    "No action is required for this check.",
                    domain::Confidence::medium,
                    true,
                    "docs/supported-platforms.md"};
  for (const auto &module : s.dkms.failed_modules) {
    f.evidence.push_back({"failed module", module});
  }
  for (const auto &error : s.dkms.parse_errors) {
    f.evidence.push_back({"parse warning", error});
  }
  for (const auto &module : s.dkms.incomplete_modules) {
    f.evidence.push_back({"incomplete module", module});
  }
  if (s.dkms.command_missing) {
    f.status = domain::CheckStatus::unknown;
    f.severity = domain::Severity::warning;
    f.explanation = "The DKMS tool is unavailable, so registered module state is unknown.";
    f.recommendation = "Confirm whether DKMS-managed modules are present before upgrading.";
    f.collector_complete = false;
  } else if (!s.dkms.failed_modules.empty()) {
    f.status = domain::CheckStatus::blocked;
    f.severity = domain::Severity::blocker;
    f.explanation = "Failed or incomplete DKMS modules were detected.";
    f.recommendation = "Resolve DKMS module build/install failures before upgrading.";
  } else if (!s.dkms.incomplete_modules.empty()) {
    f.status = domain::CheckStatus::blocked;
    f.severity = domain::Severity::blocker;
    f.explanation = "DKMS modules are registered but not installed for one or more kernels.";
    f.recommendation = "Resolve DKMS added/built states before upgrading.";
  } else if (!s.dkms.parse_errors.empty()) {
    f.status = domain::CheckStatus::unknown;
    f.severity = domain::Severity::warning;
    f.explanation = "DKMS output included malformed lines.";
    f.recommendation = "Inspect dkms status output manually.";
    f.collector_complete = false;
  }
  return f;
}

} // namespace

std::vector<std::unique_ptr<ports::IReadinessRule>> make_kernel_rules() {
  std::vector<std::unique_ptr<ports::IReadinessRule>> rules;
  rules.push_back(std::make_unique<Rule>("UG-DKM-001", dkms));
  rules.push_back(std::make_unique<Rule>("UG-KRN-001", fallback));
  rules.push_back(std::make_unique<Rule>("UG-KRN-002", consistency));
  return rules;
}

} // namespace upgrade_guard::modules
