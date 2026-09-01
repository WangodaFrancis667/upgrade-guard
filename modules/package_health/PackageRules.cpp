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

domain::Finding make(std::string id, std::string title) {
  return {std::move(id), domain::CheckStatus::passed, domain::Severity::info, std::move(title), "",
          {}, "No action is required for this check.", domain::Confidence::medium, true, "docs/rule-authoring.md"};
}

std::vector<domain::Evidence> list_evidence(const std::string &label, const std::vector<std::string> &items) {
  std::vector<domain::Evidence> evidence;
  for (const auto &item : items) {
    evidence.push_back({label, item});
  }
  return evidence;
}

domain::Finding database(const domain::SystemSnapshot &s) {
  auto f = make("UG-APT-001", "Package database completeness");
  f.evidence = {{"dpkg audit entries", std::to_string(s.packages.dpkg_audit.size())},
                {"simulation incomplete", s.packages.simulation_incomplete ? "true" : "false"}};
  if (!s.packages.dpkg_audit.empty()) {
    f.status = domain::CheckStatus::blocked;
    f.severity = domain::Severity::blocker;
    f.explanation = "dpkg reports packages in an incomplete or inconsistent state.";
    f.recommendation = "Inspect dpkg --audit output and repair package configuration before release upgrade.";
  } else if (!s.packages.simulation_performed || s.packages.simulation_incomplete) {
    f.status = domain::CheckStatus::unknown;
    f.severity = domain::Severity::warning;
    f.explanation = "APT simulation did not complete, so package database health is incomplete.";
    f.recommendation = "Review APT errors and rerun the scan.";
    f.collector_complete = false;
  } else {
    f.explanation = "dpkg audit and current-release simulation did not expose database inconsistency.";
  }
  return f;
}

domain::Finding broken(const domain::SystemSnapshot &s) {
  auto f = make("UG-APT-002", "Broken dependencies");
  f.evidence = list_evidence("broken package", s.packages.broken_packages);
  if (!s.packages.broken_packages.empty()) {
    f.status = domain::CheckStatus::blocked;
    f.severity = domain::Severity::blocker;
    f.explanation = "Broken package dependencies are present.";
    f.recommendation = "Resolve broken dependencies before attempting an OS release upgrade.";
  } else if (!s.packages.simulation_performed || s.packages.simulation_incomplete) {
    f.status = domain::CheckStatus::unknown;
    f.severity = domain::Severity::warning;
    f.explanation = "Dependency evidence is incomplete because the APT simulation did not complete.";
    f.recommendation = "Review APT errors and rerun the scan.";
    f.collector_complete = false;
  } else {
    f.explanation = "No broken dependency evidence was collected.";
  }
  return f;
}

domain::Finding holds(const domain::SystemSnapshot &s) {
  auto f = make("UG-APT-003", "Explicit package holds");
  f.evidence = list_evidence("held package", s.packages.held_packages);
  if (!s.packages.held_packages.empty()) {
    f.status = domain::CheckStatus::warning;
    f.severity = domain::Severity::warning;
    f.explanation = "Held packages can prevent release-upgrade dependency resolution.";
    f.recommendation = "Review apt-mark showhold and decide whether each hold is still intentional.";
  } else {
    f.explanation = "No explicit package holds were reported.";
  }
  return f;
}

domain::Finding third_party(const domain::SystemSnapshot &s) {
  auto f = make("UG-APT-004", "Third-party repositories");
  f.evidence = list_evidence("source", s.packages.third_party_sources);
  f.confidence = s.packages.third_party_sources.empty() ? domain::Confidence::medium : domain::Confidence::low;
  if (!s.packages.third_party_sources.empty()) {
    f.status = domain::CheckStatus::warning;
    f.severity = domain::Severity::warning;
    f.explanation = "Enabled repositories outside Ubuntu or Canonical were detected by source text classification.";
    f.recommendation = "Review third-party repository compatibility with the target release.";
  } else {
    f.explanation = "No enabled third-party repositories were detected by source parsing.";
  }
  return f;
}

domain::Finding duplicates(const domain::SystemSnapshot &s) {
  auto f = make("UG-APT-005", "Duplicate repository definitions");
  f.evidence = list_evidence("duplicate source", s.packages.duplicate_sources);
  if (!s.packages.duplicate_sources.empty()) {
    f.status = domain::CheckStatus::warning;
    f.severity = domain::Severity::warning;
    f.explanation = "Duplicate APT source definitions can confuse package policy and upgrades.";
    f.recommendation = "Review APT source files and remove duplicate definitions manually if appropriate.";
  } else {
    f.explanation = "No duplicate source definitions were detected.";
  }
  return f;
}

domain::Finding removals(const domain::SystemSnapshot &s) {
  auto f = make("UG-APT-006", "Risky package removals in current simulation");
  f.evidence = list_evidence("proposed removal", s.packages.proposed_removals);
  if (!s.packages.simulation_performed || s.packages.simulation_incomplete) {
    f.status = domain::CheckStatus::unknown;
    f.severity = domain::Severity::warning;
    f.explanation = "Removal evidence is incomplete because the current-release simulation did not complete.";
    f.recommendation = "Review APT errors and rerun the scan.";
    f.collector_complete = false;
  } else if (!s.packages.essential_removals.empty()) {
    f.status = domain::CheckStatus::blocked;
    f.severity = domain::Severity::blocker;
    f.explanation = "The current-release simulation proposes removal of essential upgrade plumbing.";
    f.recommendation = "Investigate package policy before considering a release upgrade.";
  } else if (!s.packages.proposed_removals.empty()) {
    f.status = domain::CheckStatus::warning;
    f.severity = domain::Severity::warning;
    f.explanation = "The current-release simulation proposes package removals.";
    f.recommendation = "Review the proposed removals and confirm they are expected.";
  } else {
    f.explanation = "No risky removals were detected in the current-release simulation.";
  }
  return f;
}

domain::Finding cache(const domain::SystemSnapshot &s) {
  auto f = make("UG-APT-007", "APT cache freshness");
  f.evidence = {{"cache available", s.packages.apt_cache_available ? "true" : "false"},
                {"freshness known", s.packages.apt_cache_fresh_known ? "true" : "false"}};
  if (!s.packages.apt_cache_fresh_known || s.packages.apt_cache_stale) {
    f.status = domain::CheckStatus::unknown;
    f.severity = domain::Severity::warning;
    f.explanation = "Repository metadata freshness could not be established from read-only evidence.";
    f.recommendation = "Ensure package metadata is intentionally current before upgrading.";
    f.collector_complete = false;
  } else {
    f.explanation = "APT metadata exists; freshness is only a read-only heuristic.";
  }
  return f;
}

} // namespace

std::vector<std::unique_ptr<ports::IReadinessRule>> make_package_rules() {
  std::vector<std::unique_ptr<ports::IReadinessRule>> rules;
  for (const auto &entry : std::vector<std::pair<std::string, Eval>>{{"UG-APT-001", database},
                                                                     {"UG-APT-002", broken},
                                                                     {"UG-APT-003", holds},
                                                                     {"UG-APT-004", third_party},
                                                                     {"UG-APT-005", duplicates},
                                                                     {"UG-APT-006", removals},
                                                                     {"UG-APT-007", cache}}) {
    rules.push_back(std::make_unique<Rule>(entry.first, entry.second));
  }
  return rules;
}

} // namespace upgrade_guard::modules
