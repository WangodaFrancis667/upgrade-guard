#include "upgrade_guard/application/ScanService.hpp"
#include "upgrade_guard/modules/RuleFactories.hpp"

#include "../TestSupport.hpp"
#include "../fakes/FakeProcessRunner.hpp"

#include <algorithm>

namespace upgrade_guard::tests {

namespace {

domain::SystemSnapshot collect_platform(const std::string &fixture, const std::string &target) {
  domain::SystemSnapshot snapshot;
  snapshot.request.target_release = target;
  const auto root = std::string(UPGRADE_GUARD_SOURCE_DIR) + "/tests/fixtures/" + fixture;
  const auto collector = modules::make_platform_collector(root);
  const auto result = collector->collect(snapshot);
  require(result.ok(), fixture + " platform fixture collects");
  return snapshot;
}

void platform_matrix() {
  auto ubuntu22 = collect_platform("ubuntu-22.04", "24.04");
  require(ubuntu22.platform->supported_upgrade_path, "Ubuntu 22.04 to 24.04 is supported");
  auto ubuntu24 = collect_platform("ubuntu-24.04", "26.04");
  require(ubuntu24.platform->supported_upgrade_path, "Ubuntu 24.04 to 26.04 is supported");
  auto ubuntu26 = collect_platform("ubuntu-26.04", "26.04");
  require(ubuntu26.platform->supported_current_release, "Ubuntu 26.04 fixture is recognized");
  require(ubuntu26.platform->distribution.codename == "resolute", "Ubuntu 26.04 codename fixture is resolute");
  auto pop = collect_platform("pop-os-24.04", "26.04");
  require(pop.platform->detection_only && !pop.platform->supported_upgrade_path,
          "Pop!_OS remains detection-only");
}

} // namespace

void run_fixture_scan_tests() {
  platform_matrix();
  const std::string fixture_root = std::string(UPGRADE_GUARD_SOURCE_DIR) + "/tests/fixtures/ubuntu-24.04";
  FakeProcessRunner runner;
  ports::ProcessResult ok;
  ok.exit_code = 0;
  runner.add("apt-get", ok);
  runner.add("dpkg", ok);
  ok.stdout_text = "libssl-dev\n";
  runner.add("apt-mark", ok);
  ok.stdout_text = "linux-image-a\nlinux-image-b\n";
  runner.add("dpkg-query", ok);
  ok.stdout_text.clear();
  runner.add("dkms", ok);
  ok.stdout_text = "SecureBoot disabled\n";
  runner.add("mokutil", ok);

  std::vector<std::unique_ptr<ports::ICollector>> collectors;
  collectors.push_back(modules::make_platform_collector(fixture_root));
  collectors.push_back(modules::make_apt_collector(runner, fixture_root));
  collectors.push_back(modules::make_dpkg_collector(runner));
  collectors.push_back(modules::make_sources_collector(fixture_root));

  std::vector<std::unique_ptr<ports::IReadinessRule>> rules = modules::make_platform_rules();
  auto package = modules::make_package_rules();
  for (auto &rule : package) {
    rules.push_back(std::move(rule));
  }

  std::vector<std::reference_wrapper<const ports::ICollector>> collector_refs;
  for (const auto &collector : collectors) {
    collector_refs.push_back(*collector);
  }
  std::vector<std::reference_wrapper<const ports::IReadinessRule>> rule_refs;
  for (const auto &rule : rules) {
    rule_refs.push_back(*rule);
  }
  application::ScanService service(collector_refs, rule_refs);
  const auto report = service.execute({"26.04", false});
  require(report.ok(), "fixture scan executes");
  require(report.value().platform.distribution.id == "ubuntu", "ubuntu fixture detected");
  require(report.value().overall_status != domain::ReadinessStatus::ready, "held package fixture warns");
  const auto &findings = report.value().findings;
  require(std::any_of(findings.begin(), findings.end(), [](const auto &finding) {
            return finding.id == "UG-APT-004" && finding.status == domain::CheckStatus::warning;
          }),
          "credential-bearing third-party source is classified");
  require(std::any_of(findings.begin(), findings.end(), [](const auto &finding) {
            return finding.id == "UG-APT-005" && finding.status == domain::CheckStatus::warning;
          }),
          "duplicate traditional source is classified");
}

} // namespace upgrade_guard::tests

int main() {
  upgrade_guard::tests::run_fixture_scan_tests();
  return upgrade_guard::tests::done();
}
