#include "upgrade_guard/modules/RuleFactories.hpp"

#include <sys/statvfs.h>
#include <sys/stat.h>
#include <cerrno>

namespace upgrade_guard::modules {
namespace {

domain::SpaceFacts probe(const std::string &root, const std::string &mount, dev_t root_device) {
  domain::SpaceFacts facts;
  facts.mount_point = mount;
  struct statvfs data {};
  const auto path = root == "/" ? mount : root + mount;
  if (statvfs(path.c_str(), &data) != 0) {
    facts.read_error = errno != ENOENT;
    return facts;
  }
  facts.available = true;
  facts.free_bytes = static_cast<std::uintmax_t>(data.f_bavail) * static_cast<std::uintmax_t>(data.f_frsize);
  facts.total_bytes = static_cast<std::uintmax_t>(data.f_blocks) * static_cast<std::uintmax_t>(data.f_frsize);
  struct stat info {};
  facts.separate_mount = mount == "/" || (stat(path.c_str(), &info) == 0 && info.st_dev != root_device);
  return facts;
}

class StorageCollector final : public ports::ICollector {
public:
  explicit StorageCollector(std::string root) : root_(std::move(root)) {}
  [[nodiscard]] std::string name() const override { return "storage"; }
  [[nodiscard]] domain::Result<void> collect(domain::SystemSnapshot &snapshot) const override {
    const auto root_path = root_ == "/" ? "/" : root_;
    struct stat root_info {};
    const dev_t root_device = stat(root_path.c_str(), &root_info) == 0 ? root_info.st_dev : 0;
    snapshot.storage.root = probe(root_, "/", root_device);
    snapshot.storage.boot = probe(root_, "/boot", root_device);
    snapshot.storage.efi = probe(root_, "/boot/efi", root_device);
    snapshot.storage.var = probe(root_, "/var", root_device);
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
