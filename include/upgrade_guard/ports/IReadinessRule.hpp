#pragma once

#include "upgrade_guard/domain/Finding.hpp"
#include "upgrade_guard/domain/SystemSnapshot.hpp"

namespace upgrade_guard::ports {

class IReadinessRule {
public:
  virtual ~IReadinessRule() = default;
  [[nodiscard]] virtual std::string id() const = 0;
  [[nodiscard]] virtual domain::Finding evaluate(const domain::SystemSnapshot &snapshot) const = 0;
};

} // namespace upgrade_guard::ports
