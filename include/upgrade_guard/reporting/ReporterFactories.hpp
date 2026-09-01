#pragma once

#include "upgrade_guard/ports/IReporter.hpp"

#include <memory>

namespace upgrade_guard::reporting {

std::unique_ptr<ports::IReporter> make_text_reporter(bool color);
std::unique_ptr<ports::IReporter> make_json_reporter();

} // namespace upgrade_guard::reporting
