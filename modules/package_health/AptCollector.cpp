#include "upgrade_guard/modules/RuleFactories.hpp"

#include <filesystem>
#include <sstream>

namespace upgrade_guard::modules {
namespace {

std::vector<std::string> parse_removals(const std::string &text) {
  std::vector<std::string> removals;
  std::istringstream in(text);
  std::string line;
  bool section = false;
  while (std::getline(in, line)) {
    if (line.rfind("The following packages will be REMOVED:", 0) == 0) {
      section = true;
      continue;
    }
    if (section && line.rfind("  ", 0) == 0) {
      std::istringstream words(line);
      std::string word;
      while (words >> word) {
        removals.push_back(word);
      }
    } else if (section) {
      break;
    }
  }
  return removals;
}

class AptCollector final : public ports::ICollector {
public:
  AptCollector(const ports::IProcessRunner &runner, std::string root) : runner_(runner), root_(std::move(root)) {}
  [[nodiscard]] std::string name() const override { return "apt"; }

  [[nodiscard]] domain::Result<void> collect(domain::SystemSnapshot &snapshot) const override {
    const auto lists = std::filesystem::path(root_) / "var/lib/apt/lists";
    snapshot.packages.apt_cache_available = std::filesystem::exists(lists);
    snapshot.packages.apt_cache_fresh_known = snapshot.packages.apt_cache_available;
    if (snapshot.packages.apt_cache_available) {
      snapshot.packages.apt_cache_stale = std::filesystem::is_empty(lists);
    }

    auto sim = runner_.run({"apt-get", {"--simulate", "dist-upgrade"}, std::chrono::milliseconds(8000), 131072, {}});
    if (sim.ok() && !sim.value().spawn_failed && !sim.value().timed_out) {
      snapshot.packages.simulation_performed = sim.value().exit_code == 0;
      snapshot.packages.proposed_removals = parse_removals(sim.value().stdout_text);
      for (const auto &name : snapshot.packages.proposed_removals) {
        if (name == "apt" || name == "dpkg" || name == "systemd" || name == "ubuntu-minimal") {
          snapshot.packages.essential_removals.push_back(name);
        }
      }
      snapshot.packages.simulation_incomplete = sim.value().truncated || sim.value().exit_code != 0;
    } else {
      snapshot.packages.simulation_incomplete = true;
      domain::add_issue(snapshot, name(), sim.ok() ? sim.value().failure_message : sim.error().message);
    }
    return {};
  }

private:
  const ports::IProcessRunner &runner_;
  std::string root_;
};

} // namespace

std::unique_ptr<ports::ICollector> make_apt_collector(const ports::IProcessRunner &runner, std::string root) {
  return std::make_unique<AptCollector>(runner, std::move(root));
}

} // namespace upgrade_guard::modules
