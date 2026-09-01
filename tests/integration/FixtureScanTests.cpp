#include "upgrade_guard/application/ScanService.hpp"
#include "upgrade_guard/modules/RuleFactories.hpp"

#include "../TestSupport.hpp"
#include "../fakes/FakeProcessRunner.hpp"

namespace upgrade_guard::tests {

void run_fixture_scan_tests() {
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
}

} // namespace upgrade_guard::tests
