#pragma once

#include <iostream>
#include <stdexcept>
#include <string>

namespace upgrade_guard::tests {

inline int failures = 0;

inline void require(bool condition, const std::string &message) {
  if (!condition) {
    ++failures;
    std::cerr << "FAIL: " << message << "\n";
  }
}

inline int done() { return failures == 0 ? 0 : 1; }

} // namespace upgrade_guard::tests
