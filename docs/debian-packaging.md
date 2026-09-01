# Debian packaging

The package is non-native `3.0 (quilt)`, debhelper compatibility 13, `Architecture: any`, `Rules-Requires-Root: no`, section `admin`, priority `optional`, and uses hardening flags. CMake installs the executable, man page, Bash completion, versioned JSON schema and project documentation. Runtime dependencies are generated through `${shlibs:Depends}` and `${misc:Depends}`.

`scripts/build-deb.sh` builds an unsigned binary package. `scripts/create-orig-tarball.sh` creates a deterministic upstream tarball without `.git`, Debian packaging, builds, Docker artifacts or generated reports. `scripts/build-source-package.sh` creates noble or resolute source-only artifacts and signs them only when `--key` is provided. It refuses a dirty tree unless `--force` is explicit and never calls `dput`.

Package installation/removal tests must run only in a disposable container or VM. `scripts/test-installed-package.sh` enforces `UPGRADE_GUARD_DISPOSABLE=1` before changing package state.
