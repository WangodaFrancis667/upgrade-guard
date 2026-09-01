#include "upgrade_guard/modules/RuleFactories.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <map>
#include <set>
#include <sstream>

namespace upgrade_guard::modules {
namespace {

bool third_party(const std::string &line) {
  return line.find("ppa.launchpadcontent.net") != std::string::npos ||
         (line.find("ubuntu.com") == std::string::npos && line.find("archive.canonical.com") == std::string::npos);
}

void inspect_line(const std::string &line, domain::PackageFacts &facts, std::set<std::string> &seen) {
  const bool disabled = line.rfind("#", 0) == 0;
  std::string active = disabled ? line.substr(1) : line;
  if (active.rfind("deb ", 0) != 0 && active.rfind("URIs:", 0) != 0) {
    return;
  }
  if (disabled) {
    facts.disabled_sources.push_back(active);
    return;
  }
  if (third_party(active)) {
    facts.third_party_sources.push_back(active);
  }
  if (!seen.insert(active).second) {
    facts.duplicate_sources.push_back(active);
  }
}

void read_file(const std::filesystem::path &path, domain::PackageFacts &facts, std::set<std::string> &seen) {
  std::ifstream in(path);
  std::string line;
  while (std::getline(in, line)) {
    inspect_line(line, facts, seen);
  }
}

class SourcesCollector final : public ports::ICollector {
public:
  explicit SourcesCollector(std::string root) : root_(std::move(root)) {}
  [[nodiscard]] std::string name() const override { return "apt-sources"; }
  [[nodiscard]] domain::Result<void> collect(domain::SystemSnapshot &snapshot) const override {
    std::set<std::string> seen;
    const auto etc = std::filesystem::path(root_) / "etc/apt";
    read_file(etc / "sources.list", snapshot.packages, seen);
    const auto dir = etc / "sources.list.d";
    if (std::filesystem::exists(dir)) {
      for (const auto &entry : std::filesystem::directory_iterator(dir)) {
        if (entry.path().extension() == ".list" || entry.path().extension() == ".sources") {
          read_file(entry.path(), snapshot.packages, seen);
        }
      }
    }
    return {};
  }

private:
  std::string root_;
};

} // namespace

std::unique_ptr<ports::ICollector> make_sources_collector(std::string root) {
  return std::make_unique<SourcesCollector>(std::move(root));
}

} // namespace upgrade_guard::modules
