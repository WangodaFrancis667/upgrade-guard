#pragma once

#include <string>

namespace upgrade_guard::reporting {

[[nodiscard]] std::string redact_sensitive(std::string value);

} // namespace upgrade_guard::reporting
