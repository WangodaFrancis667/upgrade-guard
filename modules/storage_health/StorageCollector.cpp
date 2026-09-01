#include "upgrade_guard/modules/RuleFactories.hpp"

#include <sys/statvfs.h>

namespace upgrade_guard::modules {
namespace {

domain::SpaceFacts probe(const std::string &root, const std::string &mount) {
  domain::SpaceFacts facts;
  facts.mount_point = mount;
  struct statvfs data {};
  const auto path = root == "/" ? mount : root + mount;
  if (statvfs(path.c_str(), &data) != 0) {
    facts.read_error = true;
    return facts;
  }
  facts.available = true;
  facts.free_bytes = static_cast<std::uintmax_t>(data.f_bavail) * static_cast<std::uintmax_t>(data.f_frsize);
  facts.separate_mount = mount == "/" || mount == "/boot" || mount == "/boot/efi" || mount == "/var";
  return facts;
}

class StorageCollector final : public ports::ICollector {
public:
  explicit StorageCollector(std::string root) : root_(std::move(root)) {}
  [[nodiscard]] std::string name() const override { return "storage"; }
  [[nodiscard]] domain::Result<void> collect(domain::SystemSnapshot &snapshot) const override {
    snapshot.storage.root = probe(root_, "/");
    snapshot.storage.boot = probe(root_, "/boot");
    snapshot.storage.efi = probe(root_, "/boot/efi");
    snapshot.storage.var = probe(root_, "/var");
    return {};
  }

private:
  std::string root_;
};

} // namespace

std::unique_ptr<ports::ICollector> make_storage_collector(std::string root) {
  return std::make_unique<StorageCollector>(std::move(root));
}

} // namespace upgrade_guard::modules
