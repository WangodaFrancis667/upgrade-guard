#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace upgrade_guard::domain {

inline constexpr const char *ToolVersion = "0.1.0-beta.1";

enum class Severity { info, warning, blocker };
enum class CheckStatus { passed, warning, blocked, unknown, error };
enum class ReadinessStatus { ready, ready_with_warnings, blocked, incomplete };
enum class Confidence { low, medium, high };

struct Evidence {
  std::string label;
  std::string value;
};

struct Distribution {
  std::string id;
  std::string name;
  std::string version_id;
  std::string codename;
  bool ubuntu_derivative{false};
};

struct PlatformFacts {
  Distribution distribution;
  std::string architecture;
  std::string kernel_version;
  bool supported_current_release{false};
  bool supported_upgrade_path{false};
  bool detection_only{false};
};

struct PackageFacts {
  bool apt_cache_available{false};
  bool apt_cache_fresh_known{false};
  bool apt_cache_stale{false};
  std::vector<std::string> broken_packages;
  std::vector<std::string> held_packages;
  std::vector<std::string> dpkg_audit;
  std::vector<std::string> duplicate_sources;
  std::vector<std::string> disabled_sources;
  std::vector<std::string> third_party_sources;
  std::vector<std::string> proposed_removals;
  std::vector<std::string> essential_removals;
  bool simulation_performed{false};
  bool simulation_incomplete{false};
};

struct SpaceFacts {
  std::string mount_point;
  std::uintmax_t free_bytes{0};
  bool available{false};
  bool separate_mount{false};
  bool read_error{false};
};

struct StorageFacts {
  SpaceFacts root;
  SpaceFacts boot;
  SpaceFacts efi;
  SpaceFacts var;
};

struct KernelFacts {
  std::string running_kernel;
  std::vector<std::string> installed_kernels;
  std::vector<std::string> missing_headers;
  std::vector<std::string> initramfs_issues;
  bool has_fallback_kernel{false};
};

struct DkmsFacts {
  bool installed{false};
  bool command_missing{false};
  std::vector<std::string> modules;
  std::vector<std::string> failed_modules;
  std::vector<std::string> parse_errors;
};

struct SecureBootFacts {
  bool mokutil_installed{false};
  bool enabled{false};
  bool state_unknown{true};
  std::string raw_state;
};

struct RebootFacts {
  bool reboot_required{false};
  std::vector<std::string> packages;
  bool read_error{false};
};

struct ScanRequest {
  std::string target_release{"26.04"};
  bool verbose{false};
};

} // namespace upgrade_guard::domain
