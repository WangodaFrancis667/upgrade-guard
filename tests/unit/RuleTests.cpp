#include "upgrade_guard/modules/RuleFactories.hpp"

#include "../TestSupport.hpp"

namespace upgrade_guard::tests {
namespace {

domain::SystemSnapshot base_snapshot() {
  domain::SystemSnapshot s;
  s.request.target_release = "26.04";
  domain::PlatformFacts p;
  p.distribution.id = "ubuntu";
  p.distribution.name = "Ubuntu 24.04 LTS";
  p.distribution.version_id = "24.04";
  p.architecture = "x86_64";
  p.supported_current_release = true;
  p.supported_upgrade_path = true;
  s.platform = p;
  s.packages.apt_cache_available = true;
  s.packages.apt_cache_fresh_known = true;
  s.storage.root = {"/", 10ULL * 1024ULL * 1024ULL * 1024ULL, true, true, false};
  s.storage.boot = {"/boot", 1024ULL * 1024ULL * 1024ULL, true, true, false};
  s.storage.efi = {"/boot/efi", 256ULL * 1024ULL * 1024ULL, true, true, false};
  s.kernel.installed_kernels = {"linux-image-a", "linux-image-b"};
  s.kernel.has_fallback_kernel = true;
  s.secure_boot.state_unknown = false;
  return s;
}

void package_rules_cover_findings() {
  auto s = base_snapshot();
  s.packages.held_packages = {"openssl"};
  s.packages.third_party_sources = {"deb https://token:secret@example.test stable main"};
  s.packages.duplicate_sources = {"deb http://archive.ubuntu.com/ubuntu noble main"};
  s.packages.essential_removals = {"apt"};
  s.packages.proposed_removals = {"apt"};
  auto rules = modules::make_package_rules();
  bool saw_warning = false;
  bool saw_blocker = false;
  for (const auto &rule : rules) {
    const auto finding = rule->evaluate(s);
    saw_warning = saw_warning || finding.status == domain::CheckStatus::warning;
    saw_blocker = saw_blocker || finding.status == domain::CheckStatus::blocked;
  }
  require(saw_warning, "package rules produce warning evidence");
  require(saw_blocker, "package rules produce blocker evidence");
}

void missing_evidence_is_not_pass() {
  auto s = base_snapshot();
  s.platform.reset();
  auto rules = modules::make_platform_rules();
  require(rules.front()->evaluate(s).status == domain::CheckStatus::unknown, "missing platform is unknown");
}

void every_required_id_exists() {
  std::vector<std::unique_ptr<ports::IReadinessRule>> rules;
  auto append = [&rules](auto more) {
    for (auto &rule : more) {
      rules.push_back(std::move(rule));
    }
  };
  append(modules::make_platform_rules());
  append(modules::make_package_rules());
  append(modules::make_storage_rules());
  append(modules::make_kernel_rules());
  append(modules::make_security_rules());
  append(modules::make_reboot_rules());
  require(rules.size() == 18, "all 18 required rule IDs are registered");
}

} // namespace
} // namespace upgrade_guard::tests

int main();

namespace upgrade_guard::tests {
void run_rule_tests() {
  package_rules_cover_findings();
  missing_evidence_is_not_pass();
  every_required_id_exists();
}
} // namespace upgrade_guard::tests
