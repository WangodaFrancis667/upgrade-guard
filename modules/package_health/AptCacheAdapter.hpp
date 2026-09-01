#pragma once

#include "upgrade_guard/domain/Result.hpp"

#include <string>
#include <vector>

namespace upgrade_guard::modules::apt_cache {

struct Evidence {
  bool adapter_compiled{false};
  bool cache_opened{false};
  unsigned long broken_count{0};
  std::vector<std::string> upgradable_packages;
};

[[nodiscard]] domain::Result<Evidence> inspect();

} // namespace upgrade_guard::modules::apt_cache
