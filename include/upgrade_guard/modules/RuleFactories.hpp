#pragma once

#include "upgrade_guard/ports/ICollector.hpp"
#include "upgrade_guard/ports/IProcessRunner.hpp"
#include "upgrade_guard/ports/IReadinessRule.hpp"

#include <memory>
#include <string>
#include <vector>

namespace upgrade_guard::modules {

std::vector<std::unique_ptr<ports::IReadinessRule>> make_platform_rules();
std::vector<std::unique_ptr<ports::IReadinessRule>> make_package_rules();
std::vector<std::unique_ptr<ports::IReadinessRule>> make_storage_rules();
std::vector<std::unique_ptr<ports::IReadinessRule>> make_kernel_rules();
std::vector<std::unique_ptr<ports::IReadinessRule>> make_security_rules();
std::vector<std::unique_ptr<ports::IReadinessRule>> make_reboot_rules();

std::unique_ptr<ports::ICollector> make_platform_collector(std::string root);
std::unique_ptr<ports::ICollector> make_apt_collector(const ports::IProcessRunner &runner, std::string root);
std::unique_ptr<ports::ICollector> make_dpkg_collector(const ports::IProcessRunner &runner);
std::unique_ptr<ports::ICollector> make_sources_collector(std::string root);
std::unique_ptr<ports::ICollector> make_storage_collector(std::string root);
std::unique_ptr<ports::ICollector> make_kernel_collector(const ports::IProcessRunner &runner, std::string root);
std::unique_ptr<ports::ICollector> make_dkms_collector(const ports::IProcessRunner &runner);
std::unique_ptr<ports::ICollector> make_secure_boot_collector(const ports::IProcessRunner &runner);
std::unique_ptr<ports::ICollector> make_reboot_collector(std::string root);

} // namespace upgrade_guard::modules
