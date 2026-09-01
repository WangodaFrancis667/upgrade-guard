#include "upgrade_guard/reporting/ReporterFactories.hpp"

#include "../TestSupport.hpp"

namespace upgrade_guard::tests {
void run_rule_tests();
void run_process_runner_tests();
void run_fixture_scan_tests();

namespace {

domain::ScanReport sample_report() {
  domain::ScanReport report;
  report.scan_id = "fixed";
  report.started_at = "2026-01-01T00:00:00Z";
  report.completed_at = "2026-01-01T00:00:01Z";
  report.request.target_release = "26.04";
  report.platform.distribution.id = "ubuntu";
  report.platform.distribution.name = "Ubuntu 24.04 LTS";
  report.platform.distribution.version_id = "24.04";
  report.platform.architecture = "x86_64";
  report.overall_status = domain::ReadinessStatus::ready_with_warnings;
  report.findings.push_back({"UG-APT-004",
                             domain::CheckStatus::warning,
                             domain::Severity::warning,
                             "Third-party repositories",
                             "Enabled third-party source.",
                             {{"source", "deb https://user:token@example.test/repo stable main"}},
                             "Review the repository.",
                             domain::Confidence::low,
                             true,
                             ""});
  report.limitations.push_back("Target-release dependency simulation was not performed.");
  return report;
}

void json_redacts_credentials() {
  const auto reporter = reporting::make_json_reporter();
  const auto text = reporter->format(sample_report());
  require(text.find("user:token") == std::string::npos, "json reporter redacts repository credentials");
  require(text.find("schema_version") != std::string::npos, "json contains schema version");
}

void text_mentions_no_changes() {
  const auto reporter = reporting::make_text_reporter(false);
  const auto text = reporter->format(sample_report());
  require(text.find("No system changes were made.") != std::string::npos, "text report states read-only result");
}

} // namespace

} // namespace upgrade_guard::tests

int main() {
  upgrade_guard::tests::run_rule_tests();
  upgrade_guard::tests::json_redacts_credentials();
  upgrade_guard::tests::text_mentions_no_changes();
  upgrade_guard::tests::run_process_runner_tests();
  upgrade_guard::tests::run_fixture_scan_tests();
  return upgrade_guard::tests::done();
}
