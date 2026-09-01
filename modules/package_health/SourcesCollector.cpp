#include "upgrade_guard/modules/RuleFactories.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <cctype>

namespace upgrade_guard::modules {
namespace {

bool third_party(const std::string &line) {
  return line.find("ppa.launchpadcontent.net") != std::string::npos ||
         (line.find("ubuntu.com") == std::string::npos && line.find("archive.canonical.com") == std::string::npos);
}

std::string trim(std::string value) {
  const auto first = value.find_first_not_of(" \t\r");
  if (first == std::string::npos) {
    return {};
  }
  const auto last = value.find_last_not_of(" \t\r");
  return value.substr(first, last - first + 1);
}

void inspect_line(const std::string &line, domain::PackageFacts &facts, std::set<std::string> &seen) {
  const auto cleaned = trim(line);
  const bool disabled = cleaned.rfind("#", 0) == 0;
  std::string active = trim(disabled ? cleaned.substr(1) : cleaned);
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

void read_list(const std::filesystem::path &path, domain::PackageFacts &facts, std::set<std::string> &seen) {
  std::ifstream in(path);
  std::string line;
  while (std::getline(in, line)) {
    inspect_line(line, facts, seen);
  }
}

void inspect_stanza(const std::map<std::string, std::string> &stanza, domain::PackageFacts &facts,
                    std::set<std::string> &seen) {
  if (stanza.empty() || !stanza.contains("URIs")) {
    return;
  }
  std::string normalized = "deb " + stanza.at("URIs");
  if (stanza.contains("Suites")) {
    normalized += " " + stanza.at("Suites");
  }
  if (stanza.contains("Components")) {
    normalized += " " + stanza.at("Components");
  }
  if (stanza.contains("Enabled") && trim(stanza.at("Enabled")) == "no") {
    facts.disabled_sources.push_back(normalized);
    return;
  }
  if (third_party(normalized)) {
    facts.third_party_sources.push_back(normalized);
  }
  if (!seen.insert(normalized).second) {
    facts.duplicate_sources.push_back(normalized);
  }
}

void read_deb822(const std::filesystem::path &path, domain::PackageFacts &facts, std::set<std::string> &seen) {
  std::ifstream in(path);
  std::map<std::string, std::string> stanza;
  std::string line;
  std::string key;
  while (std::getline(in, line)) {
    if (trim(line).empty()) {
      inspect_stanza(stanza, facts, seen);
      stanza.clear();
      key.clear();
      continue;
    }
    if (!key.empty() && (line.front() == ' ' || line.front() == '\t')) {
      stanza[key] += " " + trim(line);
      continue;
    }
    const auto colon = line.find(':');
    if (colon != std::string::npos) {
      key = line.substr(0, colon);
      stanza[key] = trim(line.substr(colon + 1));
    }
  }
  inspect_stanza(stanza, facts, seen);
}

class SourcesCollector final : public ports::ICollector {
public:
  explicit SourcesCollector(std::string root) : root_(std::move(root)) {}
  [[nodiscard]] std::string name() const override { return "apt-sources"; }
  [[nodiscard]] domain::Result<void> collect(domain::SystemSnapshot &snapshot) const override {
    std::set<std::string> seen;
    const auto etc = std::filesystem::path(root_) / "etc/apt";
    if (std::filesystem::exists(etc / "sources.list")) {
      read_list(etc / "sources.list", snapshot.packages, seen);
    }
    const auto dir = etc / "sources.list.d";
    if (std::filesystem::exists(dir)) {
      for (const auto &entry : std::filesystem::directory_iterator(dir)) {
        if (entry.path().extension() == ".list") {
          read_list(entry.path(), snapshot.packages, seen);
        } else if (entry.path().extension() == ".sources") {
          read_deb822(entry.path(), snapshot.packages, seen);
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
