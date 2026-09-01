#include "upgrade_guard/modules/RuleFactories.hpp"
#include "AptCacheAdapter.hpp"

#include <filesystem>
#include <sstream>
#include <chrono>

namespace upgrade_guard::modules {
namespace {

std::vector<std::string> parse_section(const std::string &text, const std::string &heading) {
  std::vector<std::string> packages;
  std::istringstream in(text);
  std::string line;
  bool section = false;
  while (std::getline(in, line)) {
    if (line.rfind(heading, 0) == 0) {
      section = true;
      continue;
    }
    if (section && line.rfind("  ", 0) == 0) {
      std::istringstream words(line);
      std::string word;
      while (words >> word) {
        packages.push_back(word);
      }
    } else if (section) {
      break;
    }
  }
  return packages;
}

std::vector<std::string> parse_inst(const std::string &text) {
  std::vector<std::string> packages;
  std::istringstream in(text);
  std::string line;
  while (std::getline(in, line)) {
    if (line.rfind("Inst ", 0) == 0) {
      std::istringstream words(line.substr(5));
      std::string package;
      if (words >> package) {
        packages.push_back(package);
      }
    }
  }
  return packages;
}

class AptCollector final : public ports::ICollector {
public:
  AptCollector(const ports::IProcessRunner &runner, std::string root) : runner_(runner), root_(std::move(root)) {}
  [[nodiscard]] std::string name() const override { return "apt"; }

  [[nodiscard]] domain::Result<void> collect(domain::SystemSnapshot &snapshot) const override {
    const auto cache_evidence = apt_cache::inspect();
    if (!cache_evidence.ok()) {
      domain::add_issue(snapshot, "libapt-pkg", cache_evidence.error().message);
    } else if (cache_evidence.value().adapter_compiled) {
      snapshot.packages.apt_cache_available = cache_evidence.value().cache_opened;
      snapshot.packages.upgradable_packages = cache_evidence.value().upgradable_packages;
      if (cache_evidence.value().broken_count != 0) {
        snapshot.packages.broken_packages.push_back(std::to_string(cache_evidence.value().broken_count) +
                                                    " package(s) marked broken by libapt-pkg");
      }
    }
    const auto lists = std::filesystem::path(root_) / "var/lib/apt/lists";
    snapshot.packages.apt_cache_available = snapshot.packages.apt_cache_available || std::filesystem::exists(lists);
    if (snapshot.packages.apt_cache_available && !std::filesystem::is_empty(lists)) {
      std::filesystem::file_time_type newest{};
      for (const auto &entry : std::filesystem::directory_iterator(lists)) {
        std::error_code error;
        const auto changed = entry.last_write_time(error);
        if (!error && changed > newest) {
          newest = changed;
        }
      }
      snapshot.packages.apt_cache_fresh_known = newest != std::filesystem::file_time_type{};
      if (snapshot.packages.apt_cache_fresh_known) {
        snapshot.packages.apt_cache_stale =
            std::filesystem::file_time_type::clock::now() - newest > std::chrono::hours(24 * 7);
      }
    }

    auto sim = runner_.run({"apt-get", {"--simulate", "--no-download", "dist-upgrade"},
                            std::chrono::milliseconds(8000), 131072, {}});
    if (sim.ok() && !sim.value().spawn_failed && !sim.value().timed_out) {
      snapshot.packages.simulation_performed = sim.value().exit_code == 0;
      snapshot.packages.proposed_removals =
          parse_section(sim.value().stdout_text, "The following packages will be REMOVED:");
      snapshot.packages.proposed_installs =
          parse_section(sim.value().stdout_text, "The following NEW packages will be installed:");
      snapshot.packages.proposed_upgrades = parse_inst(sim.value().stdout_text);
      if (snapshot.packages.upgradable_packages.empty()) {
        snapshot.packages.upgradable_packages = snapshot.packages.proposed_upgrades;
      }
      if (sim.value().stderr_text.find("Unmet dependencies") != std::string::npos) {
        snapshot.packages.broken_packages.push_back("APT reported unmet dependencies");
      }
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
