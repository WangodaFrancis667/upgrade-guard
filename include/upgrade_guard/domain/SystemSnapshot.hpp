#pragma once

#include "upgrade_guard/domain/Types.hpp"

#include <optional>
#include <string>
#include <vector>

namespace upgrade_guard::domain {

struct CollectorIssue {
  std::string collector;
  std::string message;
};

struct SystemSnapshot {
  ScanRequest request;
  std::optional<PlatformFacts> platform;
  PackageFacts packages;
  StorageFacts storage;
  KernelFacts kernel;
  DkmsFacts dkms;
  SecureBootFacts secure_boot;
  RebootFacts reboot;
  std::vector<CollectorIssue> collector_issues;
};

inline void add_issue(SystemSnapshot &snapshot, std::string collector, std::string message) {
  snapshot.collector_issues.push_back({std::move(collector), std::move(message)});
}

} // namespace upgrade_guard::domain
