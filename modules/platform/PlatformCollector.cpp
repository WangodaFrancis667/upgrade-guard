#include "upgrade_guard/modules/RuleFactories.hpp"

#include <fstream>
#include <filesystem>
#include <map>
#include <sys/utsname.h>

namespace upgrade_guard::modules {
namespace {

std::string unquote(std::string value) {
  if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
    value = value.substr(1, value.size() - 2);
  }
  return value;
}

std::map<std::string, std::string> read_os_release(const std::string &path) {
  std::ifstream in(path);
  std::map<std::string, std::string> values;
  std::string line;
  while (std::getline(in, line)) {
    const auto pos = line.find('=');
    if (pos != std::string::npos) {
      values[line.substr(0, pos)] = unquote(line.substr(pos + 1));
    }
  }
  return values;
}

bool is_container(const std::string &root) {
  namespace fs = std::filesystem;
  if (fs::exists(fs::path(root) / ".dockerenv") || fs::exists(fs::path(root) / "run/.containerenv")) {
    return true;
  }
  std::ifstream in(fs::path(root) / "proc/1/cgroup");
  std::string line;
  while (std::getline(in, line)) {
    if (line.find("docker") != std::string::npos || line.find("containerd") != std::string::npos ||
        line.find("kubepods") != std::string::npos || line.find("lxc") != std::string::npos) {
      return true;
    }
  }
  return false;
}

bool supported_current(const std::string &id, const std::string &version) {
  return id == "ubuntu" && (version == "22.04" || version == "24.04" || version == "26.04");
}

bool supported_path(const std::string &id, const std::string &from, const std::string &target) {
  return id == "ubuntu" && ((from == "24.04" && target == "26.04") || (from == "22.04" && target == "24.04"));
}

class PlatformCollector final : public ports::ICollector {
public:
  explicit PlatformCollector(std::string root) : root_(std::move(root)) {}
  [[nodiscard]] std::string name() const override { return "platform"; }

  [[nodiscard]] domain::Result<void> collect(domain::SystemSnapshot &snapshot) const override {
    const auto os = read_os_release(root_ + "/etc/os-release");
    if (os.empty()) {
      return domain::Error{"could not read os-release"};
    }
    domain::PlatformFacts facts;
    facts.distribution.id = os.contains("ID") ? os.at("ID") : "";
    facts.distribution.name = os.contains("PRETTY_NAME") ? os.at("PRETTY_NAME") : facts.distribution.id;
    facts.distribution.version_id = os.contains("VERSION_ID") ? os.at("VERSION_ID") : "";
    facts.distribution.codename = os.contains("VERSION_CODENAME") ? os.at("VERSION_CODENAME") : "";
    const auto id_like = os.contains("ID_LIKE") ? os.at("ID_LIKE") : "";
    facts.distribution.ubuntu_derivative = facts.distribution.id != "ubuntu" && id_like.find("ubuntu") != std::string::npos;
    facts.detection_only = facts.distribution.ubuntu_derivative || facts.distribution.id == "pop";
    facts.container_detected = is_container(root_);

    utsname uts{};
    if (uname(&uts) == 0) {
      facts.architecture = uts.machine;
      facts.kernel_version = uts.release;
    }
    facts.supported_current_release = supported_current(facts.distribution.id, facts.distribution.version_id);
    facts.supported_upgrade_path = supported_path(facts.distribution.id, facts.distribution.version_id, snapshot.request.target_release);
    snapshot.platform = facts;
    return {};
  }

private:
  std::string root_;
};

} // namespace

std::unique_ptr<ports::ICollector> make_platform_collector(std::string root) {
  return std::make_unique<PlatformCollector>(std::move(root));
}

} // namespace upgrade_guard::modules
