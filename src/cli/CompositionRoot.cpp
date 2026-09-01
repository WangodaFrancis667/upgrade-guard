#include "upgrade_guard/cli/CompositionRoot.hpp"
#include "upgrade_guard/modules/RuleFactories.hpp"
#include "upgrade_guard/ports/ProcessRunnerFactory.hpp"
#include "upgrade_guard/reporting/ReporterFactories.hpp"

#include <memory>

namespace upgrade_guard::cli {

void append_rules(std::vector<std::unique_ptr<ports::IReadinessRule>> &target,
                  std::vector<std::unique_ptr<ports::IReadinessRule>> source) {
  for (auto &rule : source) {
    target.push_back(std::move(rule));
  }
}

std::unique_ptr<AppGraph> make_app_graph(bool color) {
  auto graph = std::make_unique<AppGraph>();
  graph->runner = make_posix_process_runner();
  graph->collectors.push_back(modules::make_platform_collector("/"));
  graph->collectors.push_back(modules::make_apt_collector(*graph->runner, "/"));
  graph->collectors.push_back(modules::make_dpkg_collector(*graph->runner));
  graph->collectors.push_back(modules::make_sources_collector("/"));
  graph->collectors.push_back(modules::make_storage_collector("/"));
  graph->collectors.push_back(modules::make_kernel_collector(*graph->runner, "/"));
  graph->collectors.push_back(modules::make_dkms_collector(*graph->runner));
  graph->collectors.push_back(modules::make_secure_boot_collector(*graph->runner));
  graph->collectors.push_back(modules::make_reboot_collector("/"));

  append_rules(graph->rules, modules::make_platform_rules());
  append_rules(graph->rules, modules::make_package_rules());
  append_rules(graph->rules, modules::make_storage_rules());
  append_rules(graph->rules, modules::make_kernel_rules());
  append_rules(graph->rules, modules::make_security_rules());
  append_rules(graph->rules, modules::make_reboot_rules());

  std::vector<std::reference_wrapper<const ports::ICollector>> collector_refs;
  for (const auto &collector : graph->collectors) {
    collector_refs.push_back(*collector);
  }
  std::vector<std::reference_wrapper<const ports::IReadinessRule>> rule_refs;
  for (const auto &rule : graph->rules) {
    rule_refs.push_back(*rule);
  }
  graph->text_reporter = reporting::make_text_reporter(color);
  graph->json_reporter = reporting::make_json_reporter();
  graph->scan_service = std::make_unique<application::ScanService>(collector_refs, rule_refs);
  return graph;
}

} // namespace upgrade_guard::cli
