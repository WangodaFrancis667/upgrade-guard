cmake_minimum_required(VERSION 3.22)
project(upgrade_guard VERSION 0.1.0 LANGUAGES CXX)

option(UPGRADE_GUARD_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)
option(UPGRADE_GUARD_ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(UPGRADE_GUARD_ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)
option(UPGRADE_GUARD_ENABLE_COVERAGE "Enable gcov coverage instrumentation" OFF)
option(UPGRADE_GUARD_ENABLE_STATIC_ANALYSIS "Run available static analyzers during compilation" OFF)

include(CTest)
include(cmake/CompilerWarnings.cmake)
include(cmake/Sanitizers.cmake)
include(cmake/StaticAnalysis.cmake)
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(APT_PKG QUIET IMPORTED_TARGET apt-pkg)
endif()

if(UPGRADE_GUARD_ENABLE_COVERAGE)
  add_compile_options(--coverage -O0 -g)
  add_link_options(--coverage)
endif()

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
  modules/package_health/AptCacheAdapter.cpp
  modules/package_health/AptCollector.cpp
  modules/package_health/DpkgCollector.cpp
  modules/package_health/SourcesCollector.cpp
  modules/package_health/PackageRules.cpp)
target_link_libraries(upgrade_guard_package_health PUBLIC upgrade_guard_ports)
if(TARGET PkgConfig::APT_PKG)
  target_link_libraries(upgrade_guard_package_health PRIVATE PkgConfig::APT_PKG)
  target_compile_definitions(upgrade_guard_package_health PRIVATE UPGRADE_GUARD_HAVE_APT_PKG=1)
endif()
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

add_library(upgrade_guard_reporting src/reporting/TextReporter.cpp src/reporting/JsonReporter.cpp
  src/reporting/Redaction.cpp)
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
install(FILES completions/upgrade-guard DESTINATION share/bash-completion/completions)
install(FILES README.md docs/architecture.md docs/privacy.md docs/threat-model.md
  DESTINATION share/doc/upgrade-guard)

if(BUILD_TESTING)
  function(upgrade_guard_test target)
    target_link_libraries(${target} PRIVATE
    upgrade_guard_application
    upgrade_guard_infrastructure
    upgrade_guard_platform
    upgrade_guard_package_health
    upgrade_guard_storage_health
    upgrade_guard_kernel_health
    upgrade_guard_security_health
    upgrade_guard_reboot_health
    upgrade_guard_reporting)
    target_compile_definitions(${target} PRIVATE UPGRADE_GUARD_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
    upgrade_guard_set_warnings(${target})
    upgrade_guard_enable_sanitizers(${target})
  endfunction()

  add_executable(upgrade_guard_unit_tests tests/unit/RuleTests.cpp tests/unit/ReporterTests.cpp)
  upgrade_guard_test(upgrade_guard_unit_tests)
  add_test(NAME unit COMMAND upgrade_guard_unit_tests)

  add_executable(upgrade_guard_contract_tests tests/contract/ProcessRunnerTests.cpp)
  upgrade_guard_test(upgrade_guard_contract_tests)
  add_test(NAME contract COMMAND upgrade_guard_contract_tests)

  add_executable(upgrade_guard_integration_tests tests/integration/FixtureScanTests.cpp)
  upgrade_guard_test(upgrade_guard_integration_tests)
  add_test(NAME integration COMMAND upgrade_guard_integration_tests)

  add_test(NAME cli COMMAND ${CMAKE_COMMAND} -DUG_BINARY=$<TARGET_FILE:upgrade-guard>
    -DUG_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR} -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/cli/CliSmoke.cmake)
  if(UPGRADE_GUARD_ENABLE_ASAN)
    set_tests_properties(unit contract integration cli PROPERTIES ENVIRONMENT "ASAN_OPTIONS=detect_leaks=0")
  endif()
endif()
