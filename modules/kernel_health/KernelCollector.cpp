#include "upgrade_guard/modules/RuleFactories.hpp"

#include <filesystem>
#include <sstream>
#include <sys/utsname.h>

namespace upgrade_guard::modules {
namespace {

std::vector<std::string> lines(const std::string &text) {
  std::istringstream in(text);
  std::vector<std::string> result;
  std::string line;
  while (std::getline(in, line)) {
    if (!line.empty()) {
      result.push_back(line);
    }
  }
  return result;
}

class KernelCollector final : public ports::ICollector {
public:
  KernelCollector(const ports::IProcessRunner &runner, std::string root) : runner_(runner), root_(std::move(root)) {}
  [[nodiscard]] std::string name() const override { return "kernel"; }
  [[nodiscard]] domain::Result<void> collect(domain::SystemSnapshot &snapshot) const override {
    utsname uts{};
    if (uname(&uts) == 0) {
      snapshot.kernel.running_kernel = uts.release;
    }
    auto query = runner_.run({"dpkg-query",
                              {"-W", "-f=${Package}\\n", "linux-image-*"},
                              std::chrono::milliseconds(4000),
                              65536,
                              {}});
    if (query.ok() && !query.value().spawn_failed) {
      snapshot.kernel.installed_kernels = lines(query.value().stdout_text);
    }
    snapshot.kernel.has_fallback_kernel = snapshot.kernel.installed_kernels.size() > 1;
    const auto boot = std::filesystem::path(root_) / "boot";
    if (std::filesystem::exists(boot)) {
      for (const auto &entry : std::filesystem::directory_iterator(boot)) {
        const auto name = entry.path().filename().string();
        if (name.rfind("initrd.img-", 0) == 0 && std::filesystem::file_size(entry.path()) < 1024) {
          snapshot.kernel.initramfs_issues.push_back(name + " is unexpectedly small");
        }
      }
    }
    return {};
  }

private:
  const ports::IProcessRunner &runner_;
  std::string root_;
};

} // namespace

std::unique_ptr<ports::ICollector> make_kernel_collector(const ports::IProcessRunner &runner, std::string root) {
  return std::make_unique<KernelCollector>(runner, std::move(root));
}

} // namespace upgrade_guard::modules
