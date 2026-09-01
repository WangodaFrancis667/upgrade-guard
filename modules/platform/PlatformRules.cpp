#include "upgrade_guard/modules/RuleFactories.hpp"

#include <functional>

namespace upgrade_guard::modules {
namespace {

using Eval = std::function<domain::Finding(const domain::SystemSnapshot &)>;

domain::Finding base(std::string id, std::string title) {
  return {std::move(id),
          domain::CheckStatus::passed,
          domain::Severity::info,
          std::move(title),
          "",
          {},
          "No action is required for this check.",
          domain::Confidence::high,
          true,
          "docs/supported-platforms.md"};
}

class Rule final : public ports::IReadinessRule {
public:
  Rule(std::string id, Eval eval) : id_(std::move(id)), eval_(std::move(eval)) {}
  [[nodiscard]] std::string id() const override { return id_; }
  [[nodiscard]] domain::Finding evaluate(const domain::SystemSnapshot &snapshot) const override { return eval_(snapshot); }

private:
  std::string id_;
  Eval eval_;
};

domain::Finding current_release(const domain::SystemSnapshot &snapshot) {
  auto f = base("UG-REL-001", "Current release support");
  if (!snapshot.platform.has_value()) {
    f.status = domain::CheckStatus::unknown;
    f.severity = domain::Severity::warning;
    f.explanation = "Distribution evidence was unavailable.";
    f.recommendation = "Confirm /etc/os-release is readable.";
    f.collector_complete = false;
    return f;
  }
  const auto &p = snapshot.platform.value();
  f.evidence = {{"distribution", p.distribution.name}, {"version", p.distribution.version_id}};
  if (p.detection_only) {
    f.status = domain::CheckStatus::warning;
    f.severity = domain::Severity::warning;
    f.explanation = "This derivative release is detected, but not validated as a supported Ubuntu source release.";
    f.recommendation = "Use the derivative distribution's own upgrade guidance.";
  } else if (!p.supported_current_release) {
    f.status = domain::CheckStatus::blocked;
    f.severity = domain::Severity::blocker;
    f.explanation = "This Ubuntu release is outside the validated 0.1.0 support matrix.";
    f.recommendation = "Use a validated Ubuntu LTS source release or treat the report as advisory only.";
  } else {
    f.explanation = "The current release is inside the documented diagnostic support matrix.";
  }
  return f;
}

domain::Finding upgrade_path(const domain::SystemSnapshot &snapshot) {
  auto f = base("UG-REL-002", "Requested upgrade path support");
  if (!snapshot.platform.has_value()) {
    f.status = domain::CheckStatus::unknown;
    f.severity = domain::Severity::warning;
    f.collector_complete = false;
    f.explanation = "Upgrade path cannot be evaluated without platform facts.";
    return f;
  }
  const auto &p = snapshot.platform.value();
  f.evidence = {{"from", p.distribution.version_id}, {"target", snapshot.request.target_release}};
  if (p.detection_only) {
    f.status = domain::CheckStatus::warning;
    f.severity = domain::Severity::warning;
    f.explanation = "Upgrade Guard does not validate release-upgrade paths for Ubuntu derivatives.";
    f.recommendation = "Treat package, disk and kernel findings as diagnostic only.";
  } else if (!p.supported_upgrade_path) {
    f.status = domain::CheckStatus::blocked;
    f.severity = domain::Severity::blocker;
    f.explanation = "The requested path is not one of the validated Ubuntu LTS paths.";
    f.recommendation = "Choose 22.04 to 24.04 or 24.04 to 26.04 for this release.";
  } else {
    f.explanation = "The requested target matches a documented diagnostic path.";
  }
  return f;
}

domain::Finding derivative(const domain::SystemSnapshot &snapshot) {
  auto f = base("UG-REL-003", "Ubuntu derivative support");
  if (!snapshot.platform.has_value()) {
    f.status = domain::CheckStatus::unknown;
    f.severity = domain::Severity::warning;
    f.collector_complete = false;
    f.explanation = "Derivative status cannot be evaluated without platform facts.";
    return f;
  }
  const auto &p = snapshot.platform.value();
  f.evidence = {{"distribution", p.distribution.id}, {"detection_only", p.detection_only ? "true" : "false"}};
  if (p.detection_only) {
    f.status = domain::CheckStatus::warning;
    f.severity = domain::Severity::warning;
    f.explanation = "This distribution is treated as detection-only, not as a validated Ubuntu upgrade path.";
    f.recommendation = "Consult the derivative distribution's upgrade documentation before acting.";
  } else {
    f.explanation = "The distribution is not being handled as detection-only.";
  }
  return f;
}

} // namespace

std::vector<std::unique_ptr<ports::IReadinessRule>> make_platform_rules() {
  std::vector<std::unique_ptr<ports::IReadinessRule>> rules;
  rules.push_back(std::make_unique<Rule>("UG-REL-001", current_release));
  rules.push_back(std::make_unique<Rule>("UG-REL-002", upgrade_path));
  rules.push_back(std::make_unique<Rule>("UG-REL-003", derivative));
  return rules;
}

} // namespace upgrade_guard::modules
