#pragma once

#include "upgrade_guard/domain/Finding.hpp"
#include "upgrade_guard/domain/Result.hpp"
#include "upgrade_guard/ports/ICollector.hpp"
#include "upgrade_guard/ports/IReadinessRule.hpp"

#include <functional>
#include <vector>

namespace upgrade_guard::application {

class ScanService {
public:
  ScanService(std::vector<std::reference_wrapper<const ports::ICollector>> collectors,
              std::vector<std::reference_wrapper<const ports::IReadinessRule>> rules);

  [[nodiscard]] domain::Result<domain::ScanReport> execute(const domain::ScanRequest &request) const;

private:
  std::vector<std::reference_wrapper<const ports::ICollector>> collectors_;
  std::vector<std::reference_wrapper<const ports::IReadinessRule>> rules_;
};

} // namespace upgrade_guard::application
