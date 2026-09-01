#pragma once

#include "upgrade_guard/domain/Finding.hpp"

#include <string>

namespace upgrade_guard::ports {

class IReporter {
public:
  virtual ~IReporter() = default;
  [[nodiscard]] virtual std::string format(const domain::ScanReport &report) const = 0;
};

} // namespace upgrade_guard::ports
