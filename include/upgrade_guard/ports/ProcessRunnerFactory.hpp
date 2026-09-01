#pragma once

#include "upgrade_guard/ports/IProcessRunner.hpp"

#include <memory>

std::unique_ptr<upgrade_guard::ports::IProcessRunner> make_posix_process_runner();
