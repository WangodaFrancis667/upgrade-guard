#include "upgrade_guard/cli/CliApplication.hpp"

#include "upgrade_guard/cli/CompositionRoot.hpp"
#include "upgrade_guard/domain/Types.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <array>
#include <stdexcept>
#include <unistd.h>

namespace upgrade_guard::cli {

namespace {

enum ExitCode { ready = 0, warnings = 1, blocked = 2, incomplete = 3, usage = 64 };

std::string help() {
  return "Usage: upgrade-guard <command> [options]\n\n"
         "Commands:\n"
         "  scan [--target 26.04] [--format text|json] [--output FILE] [--force]\n"
         "  export --format json [--output FILE] [--force]\n"
         "  explain <finding-id>\n"
         "  list-checks\n"
         "  version\n"
         "  help\n\n"
         "Options: --target <release> --format text|json --output <file> --force --no-color --verbose --version --help\n";
}

constexpr std::array<const char *, 18> check_ids{
    "UG-REL-001", "UG-REL-002", "UG-REL-003", "UG-APT-001", "UG-APT-002", "UG-APT-003",
    "UG-APT-004", "UG-APT-005", "UG-APT-006", "UG-APT-007", "UG-DSK-001", "UG-DSK-002",
    "UG-DSK-003", "UG-DKM-001", "UG-SEC-001", "UG-KRN-001", "UG-KRN-002", "UG-RBT-001"};

bool valid_check(const std::string &id) {
  return std::find(check_ids.begin(), check_ids.end(), id) != check_ids.end();
}

bool valid_target(const std::string &target) { return target == "24.04" || target == "26.04"; }

int exit_for(domain::ReadinessStatus status) {
  switch (status) {
  case domain::ReadinessStatus::ready:
    return ready;
  case domain::ReadinessStatus::ready_with_warnings:
    return warnings;
  case domain::ReadinessStatus::blocked:
    return blocked;
  case domain::ReadinessStatus::incomplete:
    return incomplete;
  }
  return incomplete;
}

void write_output(const std::string &text, const std::string &path, bool force) {
  if (path.empty()) {
    std::cout << text;
    return;
  }
  if (std::filesystem::exists(path) && !force) {
    throw std::runtime_error("output file exists; pass --force to overwrite");
  }
  std::ofstream out(path);
  if (!out) {
    throw std::runtime_error("could not open output file: " + path);
  }
  out << text;
  if (!out) {
    throw std::runtime_error("could not write output file: " + path);
  }
}

std::string explain_text(const std::string &id) {
  if (id == "UG-APT-004") {
    return "UG-APT-004 — Third-party repositories\n"
           "Warns when an enabled APT source is outside Ubuntu or Canonical. Classification is heuristic; credentials "
           "are redacted in exported reports. Review compatibility with the target release manually.\n";
  }
  return id + " — stable readiness check. Run a verbose scan for its evidence, explanation and recommendation.\n";
}

} // namespace

int run(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << help();
    return usage;
  }
  std::string command = argv[1];
  if (command == "--help") {
    command = "help";
  }
  if (command == "--version") {
    command = "version";
  }
  if (command == "help" || (argc == 3 && std::string(argv[2]) == "--help")) {
    std::cout << help();
    return ready;
  }
  if (command == "version") {
    std::cout << "upgrade-guard " << domain::ToolVersion << "\n";
    return ready;
  }
  if (command == "explain") {
    if (argc != 3 || !valid_check(argv[2])) {
      std::cerr << "explain requires one known finding ID\n";
      return usage;
    }
    std::cout << explain_text(argv[2]);
    return ready;
  }
  if (command == "list-checks") {
    if (argc != 2) {
      return usage;
    }
    for (const auto *id : check_ids) {
      std::cout << id << "\n";
    }
    return ready;
  }
  if (command != "scan" && command != "export") {
    std::cerr << help();
    return usage;
  }

  bool color = isatty(STDOUT_FILENO) != 0 && std::getenv("NO_COLOR") == nullptr;
  bool force = false;
  domain::ScanRequest request;
  std::string format = "text";
  std::string output;
  for (int i = 2; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "--target" && i + 1 < argc) {
      request.target_release = argv[++i];
    } else if (arg == "--format" && i + 1 < argc) {
      format = argv[++i];
    } else if (arg == "--output" && i + 1 < argc) {
      output = argv[++i];
    } else if (arg == "--force") {
      force = true;
    } else if (arg == "--no-color") {
      color = false;
    } else if (arg == "--verbose") {
      request.verbose = true;
    } else {
      std::cerr << "Invalid option: " << arg << "\n";
      return usage;
    }
  }
  if (command == "export") {
    if (format != "text" && format != "json") {
      return usage;
    }
    format = "json";
  }
  if (!valid_target(request.target_release) || (format != "text" && format != "json")) {
    std::cerr << "Invalid target or format\n";
    return usage;
  }
  try {
    auto graph = make_app_graph(color);
    const auto result = graph->scan_service->execute(request);
    if (!result.ok()) {
      std::cerr << result.error().message << "\n";
      return incomplete;
    }
    const auto &report = result.value();
    const auto text = format == "json" ? graph->json_reporter->format(report) : graph->text_reporter->format(report);
    write_output(text, output, force);
    return exit_for(report.overall_status);
  } catch (const std::exception &ex) {
    std::cerr << ex.what() << "\n";
    return incomplete;
  }
}

} // namespace upgrade_guard::cli
