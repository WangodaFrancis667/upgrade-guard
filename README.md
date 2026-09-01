# Upgrade Guard

Upgrade Guard is a native C++20, read-only Ubuntu upgrade readiness assistant. It inspects evidence available on the current machine and answers whether known package, platform, storage, kernel, DKMS, Secure Boot and reboot conditions could block or complicate an Ubuntu release upgrade.

Upgrade Guard does not guarantee that an operating-system upgrade will succeed. It identifies known readiness risks from evidence available at scan time. It is not endorsed by Canonical, Ubuntu, Debian or System76.

## Commands

```bash
upgrade-guard scan
upgrade-guard scan --target 26.04
upgrade-guard scan --format json
upgrade-guard export --format json --output report.json
upgrade-guard explain UG-APT-004
upgrade-guard list-checks
upgrade-guard version
upgrade-guard help
```

Exit codes: `0` ready, `1` ready with warnings, `2` blocked, `3` incomplete scan or tool error, `64` command-line usage error.

## Safety

Version `0.1.0-beta.1` is diagnostic only. It does not run a release upgrade, install packages, remove packages, run `apt update`, edit APT sources, change Secure Boot, modify kernels, upload reports, or require sudo for ordinary scans. Every report ends with `No system changes were made.`

Target-release dependency simulation is not performed. The tool evaluates current package health, current-release `apt-get --simulate dist-upgrade`, storage, kernel, DKMS, Secure Boot, reboot state and static release policies.

## Supported Platforms

Primary diagnostic path: Ubuntu 24.04 to Ubuntu 26.04 on amd64 and arm64. Secondary path: Ubuntu 22.04 to Ubuntu 24.04. Pop!_OS 24.04 and Ubuntu derivatives are detection-only and must not be treated as validated Ubuntu upgrade paths.

## Build And Test

```bash
cmake --preset debug
cmake --build --preset debug
ctest --preset debug --output-on-failure
cmake --preset release
cmake --build --preset release
ctest --preset release --output-on-failure
```

Optional presets are available for ASan and UBSan. Debian packaging metadata is included under `debian/`.

Additional presets are `clang`, `asan`, `ubsan`, and `coverage`. Docker validation uses exact Ubuntu 22.04, 24.04 and 26.04 tags:

```bash
bash docker/test-matrix.sh
```

Docker shares the host kernel and cannot validate native kernel, DKMS, Secure Boot, EFI, reboot, bootloader or full systemd behavior. Use a disposable native VM for those checks.

Build Debian artifacts with `scripts/build-deb.sh`. Create series-specific source packages with `scripts/build-source-package.sh noble` or `scripts/build-source-package.sh resolute`. The CLI version `0.1.0-beta.1` maps to Debian upstream version `0.1.0~beta1`; source packages append the Ubuntu-series suffix documented in [docs/launchpad-release.md](docs/launchpad-release.md). No script uploads artifacts.

## Architecture

The CLI depends on the application layer, which depends on ports and domain. Linux collectors and reporters are adapters. Rules are pure strategies over an immutable `SystemSnapshot`. Concrete production dependencies are assembled only in the composition root using constructor injection.
