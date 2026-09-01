#pragma once

#include "upgrade_guard/application/ScanService.hpp"
#include "upgrade_guard/ports/ICollector.hpp"
#include "upgrade_guard/ports/IProcessRunner.hpp"
#include "upgrade_guard/ports/IReadinessRule.hpp"
#include "upgrade_guard/ports/IReporter.hpp"

#include <memory>
#include <vector>

namespace upgrade_guard::cli {

struct AppGraph {
  std::unique_ptr<ports::IProcessRunner> runner;
  std::vector<std::unique_ptr<ports::ICollector>> collectors;
  std::vector<std::unique_ptr<ports::IReadinessRule>> rules;
  std::unique_ptr<ports::IReporter> text_reporter;
  std::unique_ptr<ports::IReporter> json_reporter;
  std::unique_ptr<application::ScanService> scan_service;
};

std::unique_ptr<AppGraph> make_app_graph(bool color);

} // namespace upgrade_guard::cli
