#include "upgrade_guard/modules/RuleFactories.hpp"

#include "../TestSupport.hpp"

#include <map>
#include <functional>

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
  s.packages.simulation_performed = true;
  s.storage.root = {"/", 10ULL * 1024ULL * 1024ULL * 1024ULL, 20ULL * 1024ULL * 1024ULL * 1024ULL, true, true, false};
  s.storage.boot = {"/boot", 1024ULL * 1024ULL * 1024ULL, 2ULL * 1024ULL * 1024ULL * 1024ULL, true, true, false};
  s.storage.efi = {"/boot/efi", 256ULL * 1024ULL * 1024ULL, 512ULL * 1024ULL * 1024ULL, true, true, false};
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
  std::map<std::string, int> counts;
  for (const auto &rule : rules) {
    ++counts[rule->id()];
  }
  for (const auto &[id, count] : counts) {
    require(count == 1, id + " is registered exactly once");
  }
}

std::vector<std::unique_ptr<ports::IReadinessRule>> all_rules() {
  std::vector<std::unique_ptr<ports::IReadinessRule>> result;
  auto append = [&result](auto rules) {
    for (auto &rule : rules) {
      result.push_back(std::move(rule));
    }
  };
  append(modules::make_platform_rules());
  append(modules::make_package_rules());
  append(modules::make_storage_rules());
  append(modules::make_kernel_rules());
  append(modules::make_security_rules());
  append(modules::make_reboot_rules());
  return result;
}

domain::CheckStatus evaluate(const std::string &id, const domain::SystemSnapshot &snapshot) {
  auto rules = all_rules();
  for (const auto &rule : rules) {
    if (rule->id() == id) {
      return rule->evaluate(snapshot).status;
    }
  }
  throw std::runtime_error("missing rule " + id);
}

void rule_policy_table() {
  struct Case {
    std::string id;
    domain::CheckStatus expected;
    std::function<void(domain::SystemSnapshot &)> mutate;
  };
  const std::vector<Case> cases{
      {"UG-REL-001", domain::CheckStatus::blocked, [](auto &s) { s.platform->supported_current_release = false; }},
      {"UG-REL-002", domain::CheckStatus::blocked, [](auto &s) { s.platform->supported_upgrade_path = false; }},
      {"UG-REL-003", domain::CheckStatus::warning, [](auto &s) { s.platform->detection_only = true; }},
      {"UG-APT-001", domain::CheckStatus::blocked, [](auto &s) { s.packages.dpkg_audit = {"half-configured"}; }},
      {"UG-APT-002", domain::CheckStatus::blocked, [](auto &s) { s.packages.broken_packages = {"broken"}; }},
      {"UG-APT-003", domain::CheckStatus::warning, [](auto &s) { s.packages.held_packages = {"held"}; }},
      {"UG-APT-004", domain::CheckStatus::warning, [](auto &s) { s.packages.third_party_sources = {"source"}; }},
      {"UG-APT-005", domain::CheckStatus::warning, [](auto &s) { s.packages.duplicate_sources = {"source"}; }},
      {"UG-APT-006", domain::CheckStatus::blocked, [](auto &s) { s.packages.essential_removals = {"apt"}; }},
      {"UG-APT-007", domain::CheckStatus::unknown, [](auto &s) { s.packages.apt_cache_stale = true; }},
      {"UG-DSK-001", domain::CheckStatus::warning, [](auto &s) { s.storage.root.free_bytes = 1; }},
      {"UG-DSK-002", domain::CheckStatus::warning, [](auto &s) { s.storage.boot.free_bytes = 1; }},
      {"UG-DSK-003", domain::CheckStatus::warning, [](auto &s) { s.storage.efi.free_bytes = 1; }},
      {"UG-DKM-001", domain::CheckStatus::blocked, [](auto &s) { s.dkms.failed_modules = {"failed"}; }},
      {"UG-SEC-001", domain::CheckStatus::warning, [](auto &s) {
         s.secure_boot.enabled = true;
         s.dkms.modules = {"module"};
       }},
      {"UG-KRN-001", domain::CheckStatus::warning, [](auto &s) {
         s.kernel.installed_kernels = {"one"};
         s.kernel.has_fallback_kernel = false;
       }},
      {"UG-KRN-002", domain::CheckStatus::warning, [](auto &s) { s.kernel.missing_headers = {"kernel"}; }},
      {"UG-RBT-001", domain::CheckStatus::warning, [](auto &s) { s.reboot.reboot_required = true; }},
  };
  for (const auto &test : cases) {
    auto snapshot = base_snapshot();
    test.mutate(snapshot);
    require(evaluate(test.id, snapshot) == test.expected, test.id + " policy case");
  }
}

void incomplete_and_container_evidence_never_passes() {
  auto snapshot = base_snapshot();
  snapshot.packages.simulation_performed = false;
  require(evaluate("UG-APT-001", snapshot) == domain::CheckStatus::unknown, "missing simulation is unknown");
  require(evaluate("UG-APT-002", snapshot) == domain::CheckStatus::unknown, "missing dependency evidence is unknown");
  require(evaluate("UG-APT-006", snapshot) == domain::CheckStatus::unknown, "missing removal evidence is unknown");
  snapshot = base_snapshot();
  snapshot.dkms.command_missing = true;
  require(evaluate("UG-DKM-001", snapshot) == domain::CheckStatus::unknown, "missing DKMS tool is unknown");
  snapshot = base_snapshot();
  snapshot.platform->container_detected = true;
  require(evaluate("UG-KRN-001", snapshot) == domain::CheckStatus::unknown, "container kernel evidence is unknown");
  require(evaluate("UG-SEC-001", snapshot) == domain::CheckStatus::unknown, "container Secure Boot evidence is unknown");
}

} // namespace
} // namespace upgrade_guard::tests

int main();

namespace upgrade_guard::tests {
void run_rule_tests() {
  package_rules_cover_findings();
  missing_evidence_is_not_pass();
  every_required_id_exists();
  rule_policy_table();
  incomplete_and_container_evidence_never_passes();
}
} // namespace upgrade_guard::tests
