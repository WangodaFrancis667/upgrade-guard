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

std::string mib(std::uintmax_t bytes) { return std::to_string(bytes / 1024U / 1024U) + " MiB"; }

domain::Finding check_space(std::string id, std::string title, const domain::SpaceFacts &facts, std::uintmax_t min_bytes,
                            bool optional = false) {
  domain::Finding f{std::move(id), domain::CheckStatus::passed, domain::Severity::info, std::move(title), "",
                    {{"mount", facts.mount_point}, {"free", mib(facts.free_bytes)}, {"threshold", mib(min_bytes)}},
                    "No action is required for this check.", domain::Confidence::medium, true, "docs/supported-platforms.md"};
  if (optional && !facts.available && !facts.read_error) {
    f.explanation = "The optional filesystem was not discoverable; this check is not applicable.";
    f.evidence.push_back({"heuristic", "optional mount checked when discoverable"});
  } else if (facts.read_error || !facts.available) {
    f.status = domain::CheckStatus::unknown;
    f.severity = domain::Severity::warning;
    f.explanation = "Free space evidence could not be read.";
    f.recommendation = "Inspect mount availability and permissions.";
    f.collector_complete = false;
  } else if (facts.free_bytes < min_bytes) {
    f.status = domain::CheckStatus::warning;
    f.severity = domain::Severity::warning;
    f.explanation = "Free space is below the documented conservative heuristic threshold.";
    f.recommendation = "Investigate available space before starting a release upgrade.";
  } else {
    f.explanation = "Free space is above the documented conservative heuristic threshold.";
  }
  return f;
}

} // namespace

std::vector<std::unique_ptr<ports::IReadinessRule>> make_storage_rules() {
  constexpr std::uintmax_t mib = 1024U * 1024U;
  std::vector<std::unique_ptr<ports::IReadinessRule>> rules;
  rules.push_back(std::make_unique<Rule>("UG-DSK-001", [](const auto &s) {
    return check_space("UG-DSK-001", "Root filesystem free space", s.storage.root, 5ULL * 1024ULL * mib);
  }));
  rules.push_back(std::make_unique<Rule>("UG-DSK-002", [](const auto &s) {
    return check_space("UG-DSK-002", "Boot filesystem free space", s.storage.boot, 512ULL * mib);
  }));
  rules.push_back(std::make_unique<Rule>("UG-DSK-003", [](const auto &s) {
    return check_space("UG-DSK-003", "EFI filesystem free space", s.storage.efi, 100ULL * mib, true);
  }));
  return rules;
}

} // namespace upgrade_guard::modules
