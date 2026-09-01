cmake_minimum_required(VERSION 3.22)
project(upgrade_guard VERSION 0.1.0 LANGUAGES CXX)

option(UPGRADE_GUARD_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)
option(UPGRADE_GUARD_ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(UPGRADE_GUARD_ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)

include(CTest)
include(cmake/CompilerWarnings.cmake)
include(cmake/Sanitizers.cmake)
include(cmake/StaticAnalysis.cmake)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(upgrade_guard_domain INTERFACE)
target_include_directories(upgrade_guard_domain INTERFACE include)

add_library(upgrade_guard_ports INTERFACE)
target_link_libraries(upgrade_guard_ports INTERFACE upgrade_guard_domain)
target_include_directories(upgrade_guard_ports INTERFACE include)

add_library(upgrade_guard_application src/application/ScanService.cpp)
target_link_libraries(upgrade_guard_application PUBLIC upgrade_guard_ports)
upgrade_guard_set_warnings(upgrade_guard_application)
upgrade_guard_enable_sanitizers(upgrade_guard_application)

add_library(upgrade_guard_infrastructure src/infrastructure/PosixProcessRunner.cpp)
target_link_libraries(upgrade_guard_infrastructure PUBLIC upgrade_guard_ports)
upgrade_guard_set_warnings(upgrade_guard_infrastructure)
upgrade_guard_enable_sanitizers(upgrade_guard_infrastructure)

add_library(upgrade_guard_platform modules/platform/PlatformCollector.cpp modules/platform/PlatformRules.cpp)
target_link_libraries(upgrade_guard_platform PUBLIC upgrade_guard_ports)
upgrade_guard_set_warnings(upgrade_guard_platform)
upgrade_guard_enable_sanitizers(upgrade_guard_platform)

add_library(upgrade_guard_package_health
  modules/package_health/AptCollector.cpp
  modules/package_health/DpkgCollector.cpp
  modules/package_health/SourcesCollector.cpp
  modules/package_health/PackageRules.cpp)
target_link_libraries(upgrade_guard_package_health PUBLIC upgrade_guard_ports)
upgrade_guard_set_warnings(upgrade_guard_package_health)
upgrade_guard_enable_sanitizers(upgrade_guard_package_health)

add_library(upgrade_guard_storage_health modules/storage_health/StorageCollector.cpp modules/storage_health/StorageRules.cpp)
target_link_libraries(upgrade_guard_storage_health PUBLIC upgrade_guard_ports)
upgrade_guard_set_warnings(upgrade_guard_storage_health)
upgrade_guard_enable_sanitizers(upgrade_guard_storage_health)

add_library(upgrade_guard_kernel_health
  modules/kernel_health/KernelCollector.cpp
  modules/kernel_health/DkmsCollector.cpp
  modules/kernel_health/KernelRules.cpp)
target_link_libraries(upgrade_guard_kernel_health PUBLIC upgrade_guard_ports)
upgrade_guard_set_warnings(upgrade_guard_kernel_health)
upgrade_guard_enable_sanitizers(upgrade_guard_kernel_health)

add_library(upgrade_guard_security_health modules/security_health/SecureBootCollector.cpp modules/security_health/SecurityRules.cpp)
target_link_libraries(upgrade_guard_security_health PUBLIC upgrade_guard_ports)
upgrade_guard_set_warnings(upgrade_guard_security_health)
upgrade_guard_enable_sanitizers(upgrade_guard_security_health)

add_library(upgrade_guard_reboot_health modules/reboot_health/RebootCollector.cpp modules/reboot_health/RebootRules.cpp)
target_link_libraries(upgrade_guard_reboot_health PUBLIC upgrade_guard_ports)
upgrade_guard_set_warnings(upgrade_guard_reboot_health)
upgrade_guard_enable_sanitizers(upgrade_guard_reboot_health)

add_library(upgrade_guard_reporting src/reporting/TextReporter.cpp src/reporting/JsonReporter.cpp)
target_link_libraries(upgrade_guard_reporting PUBLIC upgrade_guard_ports)
upgrade_guard_set_warnings(upgrade_guard_reporting)
upgrade_guard_enable_sanitizers(upgrade_guard_reporting)

add_executable(upgrade-guard src/cli/main.cpp src/cli/CliApplication.cpp src/cli/CompositionRoot.cpp)
target_link_libraries(upgrade-guard PRIVATE
  upgrade_guard_application
  upgrade_guard_infrastructure
  upgrade_guard_platform
  upgrade_guard_package_health
  upgrade_guard_storage_health
  upgrade_guard_kernel_health
  upgrade_guard_security_health
  upgrade_guard_reboot_health
  upgrade_guard_reporting)
upgrade_guard_set_warnings(upgrade-guard)
upgrade_guard_enable_sanitizers(upgrade-guard)

install(TARGETS upgrade-guard RUNTIME DESTINATION bin)
install(FILES man/upgrade-guard.1 DESTINATION share/man/man1)
install(FILES schemas/report-v1.schema.json DESTINATION share/upgrade-guard/schemas)

if(BUILD_TESTING)
  add_executable(upgrade_guard_tests
    tests/unit/RuleTests.cpp
    tests/unit/ReporterTests.cpp
    tests/contract/ProcessRunnerTests.cpp
    tests/integration/FixtureScanTests.cpp)
  target_link_libraries(upgrade_guard_tests PRIVATE
    upgrade_guard_application
    upgrade_guard_infrastructure
    upgrade_guard_platform
    upgrade_guard_package_health
    upgrade_guard_storage_health
    upgrade_guard_kernel_health
    upgrade_guard_security_health
    upgrade_guard_reboot_health
    upgrade_guard_reporting)
  target_compile_definitions(upgrade_guard_tests PRIVATE UPGRADE_GUARD_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
  upgrade_guard_set_warnings(upgrade_guard_tests)
  upgrade_guard_enable_sanitizers(upgrade_guard_tests)
  add_test(NAME upgrade_guard_tests COMMAND upgrade_guard_tests)
  if(UPGRADE_GUARD_ENABLE_ASAN)
    set_tests_properties(upgrade_guard_tests PROPERTIES ENVIRONMENT "ASAN_OPTIONS=detect_leaks=0")
  endif()
endif()
