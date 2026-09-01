#include "AptCacheAdapter.hpp"

#ifdef UPGRADE_GUARD_HAVE_APT_PKG
#include <apt-pkg/cachefile.h>
#include <apt-pkg/depcache.h>
#include <apt-pkg/init.h>
#include <apt-pkg/pkgsystem.h>
#endif

namespace upgrade_guard::modules::apt_cache {

domain::Result<Evidence> inspect() {
  Evidence evidence;
#ifdef UPGRADE_GUARD_HAVE_APT_PKG
  evidence.adapter_compiled = true;
  if (!pkgInitConfig(*_config) || !pkgInitSystem(*_config, _system)) {
    return domain::Error{"libapt-pkg initialization failed"};
  }
  pkgCacheFile cache;
  if (!cache.Open()) {
    return domain::Error{"libapt-pkg could not open the package cache"};
  }
  auto *package_cache = cache.GetPkgCache();
  auto *dependency_cache = cache.GetDepCache();
  if (package_cache == nullptr || dependency_cache == nullptr) {
    return domain::Error{"libapt-pkg returned an incomplete cache"};
  }
  evidence.cache_opened = true;
  evidence.broken_count = dependency_cache->BrokenCount();
  for (auto package = package_cache->PkgBegin(); !package.end(); ++package) {
    if ((*dependency_cache)[package].Upgradable()) {
      evidence.upgradable_packages.emplace_back(package.Name());
    }
  }
#endif
  return evidence;
}

} // namespace upgrade_guard::modules::apt_cache
