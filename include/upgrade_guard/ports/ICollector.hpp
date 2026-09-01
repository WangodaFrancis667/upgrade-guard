#pragma once

#include "upgrade_guard/domain/Result.hpp"
#include "upgrade_guard/domain/SystemSnapshot.hpp"

#include <string>

namespace upgrade_guard::ports {

class ICollector {
public:
  virtual ~ICollector() = default;
  [[nodiscard]] virtual std::string name() const = 0;
  [[nodiscard]] virtual domain::Result<void> collect(domain::SystemSnapshot &snapshot) const = 0;
};

} // namespace upgrade_guard::ports
