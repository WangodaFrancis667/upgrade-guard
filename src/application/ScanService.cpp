#include "upgrade_guard/application/ScanService.hpp"

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <random>
#include <sstream>

namespace upgrade_guard::application {
namespace {

std::string now_utc() {
  const auto now = std::chrono::system_clock::now();
  const auto time = std::chrono::system_clock::to_time_t(now);
  std::tm tm{};
  gmtime_r(&time, &tm);
  std::ostringstream out;
  out << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
  return out.str();
}

std::string scan_id() {
  const auto ticks = std::chrono::steady_clock::now().time_since_epoch().count();
  std::mt19937_64 rng(static_cast<std::mt19937_64::result_type>(ticks));
  std::ostringstream out;
  out << "ug-" << std::hex << rng();
  return out.str();
}

domain::ReadinessStatus aggregate(const std::vector<domain::Finding> &findings) {
  const auto has_blocker = std::any_of(findings.begin(), findings.end(), [](const auto &finding) {
    return finding.status == domain::CheckStatus::blocked || finding.status == domain::CheckStatus::error;
  });
  if (has_blocker) {
    return domain::ReadinessStatus::blocked;
  }
  const auto incomplete = std::any_of(findings.begin(), findings.end(), [](const auto &finding) {
    return finding.status == domain::CheckStatus::unknown || !finding.collector_complete;
  });
  if (incomplete) {
    return domain::ReadinessStatus::incomplete;
  }
  const auto warnings = std::any_of(findings.begin(), findings.end(), [](const auto &finding) {
    return finding.status == domain::CheckStatus::warning;
  });
  return warnings ? domain::ReadinessStatus::ready_with_warnings : domain::ReadinessStatus::ready;
}

} // namespace

ScanService::ScanService(std::vector<std::reference_wrapper<const ports::ICollector>> collectors,
                         std::vector<std::reference_wrapper<const ports::IReadinessRule>> rules)
    : collectors_(std::move(collectors)), rules_(std::move(rules)) {}

domain::Result<domain::ScanReport> ScanService::execute(const domain::ScanRequest &request) const {
  domain::SystemSnapshot snapshot;
  snapshot.request = request;
  const auto started = now_utc();
  for (const auto &collector : collectors_) {
    const auto result = collector.get().collect(snapshot);
    if (!result.ok()) {
      domain::add_issue(snapshot, collector.get().name(), result.error().message);
    }
  }

  std::vector<domain::Finding> findings;
  findings.reserve(rules_.size() + snapshot.collector_issues.size());
  for (const auto &rule : rules_) {
    findings.push_back(rule.get().evaluate(snapshot));
  }
  for (const auto &issue : snapshot.collector_issues) {
    findings.push_back({"UG-COL-001",
                        domain::CheckStatus::unknown,
                        domain::Severity::warning,
                        "Collector returned incomplete evidence",
                        "One collector could not complete. Upgrade Guard keeps this distinct from a pass.",
                        {{"collector", issue.collector}, {"message", issue.message}},
                        "Inspect permissions and tool availability, then rerun the scan.",
                        domain::Confidence::high,
                        false,
                        "docs/architecture.md"});
  }

  domain::ScanReport report;
  report.scan_id = scan_id();
  report.started_at = started;
  report.completed_at = now_utc();
  report.request = request;
  report.findings = std::move(findings);
  report.overall_status = aggregate(report.findings);
  report.limitations.push_back("Target-release dependency simulation was not performed.");
  report.limitations.push_back("No system changes were made.");
  if (snapshot.platform.has_value()) {
    report.platform = snapshot.platform.value();
  }
  return report;
}

} // namespace upgrade_guard::application
