#pragma once

#include "upgrade_guard/domain/Types.hpp"

#include <string>
#include <vector>

namespace upgrade_guard::domain {

struct Finding {
  std::string id;
  CheckStatus status{CheckStatus::unknown};
  Severity severity{Severity::info};
  std::string title;
  std::string explanation;
  std::vector<Evidence> evidence;
  std::string recommendation;
  Confidence confidence{Confidence::medium};
  bool collector_complete{true};
  std::string documentation_ref;
};

struct ScanReport {
  std::string scan_id;
  std::string started_at;
  std::string completed_at;
  ScanRequest request;
  ReadinessStatus overall_status{ReadinessStatus::incomplete};
  std::vector<Finding> findings;
  std::vector<std::string> limitations;
  PlatformFacts platform;
};

} // namespace upgrade_guard::domain
