#include "upgrade_guard/modules/RuleFactories.hpp"

#include <filesystem>
#include <fstream>

namespace upgrade_guard::modules {
namespace {

class RebootCollector final : public ports::ICollector {
public:
  explicit RebootCollector(std::string root) : root_(std::move(root)) {}
  [[nodiscard]] std::string name() const override { return "reboot"; }
  [[nodiscard]] domain::Result<void> collect(domain::SystemSnapshot &snapshot) const override {
    const auto marker = std::filesystem::path(root_) / "var/run/reboot-required";
    std::error_code error;
    snapshot.reboot.reboot_required = std::filesystem::exists(marker, error);
    snapshot.reboot.read_error = static_cast<bool>(error);
    const auto packages = std::filesystem::path(root_) / "var/run/reboot-required.pkgs";
    if (std::filesystem::exists(packages)) {
      std::ifstream in(packages);
      if (!in) {
        snapshot.reboot.read_error = true;
      }
      std::string line;
      while (std::getline(in, line)) {
        if (!line.empty()) {
          snapshot.reboot.packages.push_back(line);
        }
      }
    }
    return {};
  }

private:
  std::string root_;
};

} // namespace

std::unique_ptr<ports::ICollector> make_reboot_collector(std::string root) {
  return std::make_unique<RebootCollector>(std::move(root));
}

} // namespace upgrade_guard::modules
