<div align="center">
  <img src="https://cdn.simpleicons.org/ubuntu/E95420" width="80" height="80" alt="Ubuntu Logo" />
  
  <h1>Upgrade Guard</h1>
  <p><em>A native C++20, read-only Ubuntu upgrade readiness assistant.</em></p>
  
  <p>
    <a href="https://github.com/wanproline/upgrade-guard/blob/main/LICENSE"><img src="https://img.shields.io/badge/License-MIT-blue.svg" alt="License: MIT" /></a>
    <img src="https://img.shields.io/badge/C%2B%2B-20-00599C?logo=c%2B%2B&logoColor=white" alt="C++20" />
    <img src="https://img.shields.io/badge/Ubuntu-24.04%20%7C%2026.04-E95420?logo=ubuntu&logoColor=white" alt="Ubuntu Support" />
  </p>
</div>

---

Upgrade Guard inspects evidence available on the current machine and answers whether known package, platform, storage, kernel, DKMS, Secure Boot and reboot conditions could block or complicate an Ubuntu release upgrade.

> [!NOTE]
> Upgrade Guard does not guarantee that an operating-system upgrade will succeed. It identifies known readiness risks from evidence available at scan time. It is not endorsed by Canonical, Ubuntu, Debian or System76.

## 🚀 Installation

You can easily install Upgrade Guard using our official PPA repository:

```bash
sudo add-apt-repository ppa:wanpro/upgrade-guard
sudo apt update
sudo apt install upgrade-guard
```

## 📖 Usage

Once installed, you can use the CLI to inspect your system's upgrade readiness. Here are the essential commands:

```bash
# Run a full upgrade readiness scan
upgrade-guard scan

# Target a specific release (e.g., Ubuntu 26.04)
upgrade-guard scan --target 26.04

# Export the scan results as JSON
upgrade-guard export --format json --output report.json

# List all available diagnostic checks
upgrade-guard list-checks

# Explain a specific diagnostic code
upgrade-guard explain UG-APT-004

# View version and help information
upgrade-guard --version
upgrade-guard --help
```

### Exit Codes

| Code | Status | Description |
| :--- | :--- | :--- |
| `0` | **Ready** | System is ready for an upgrade. |
| `1` | **Warning** | Ready with warnings. |
| `2` | **Blocked** | Upgrade is currently blocked by system conditions. |
| `3` | **Error** | Incomplete scan or tool error. |
| `64` | **Usage Error** | Command-line usage error. |

## 🛡️ Safety

Version `0.1.0-beta.1` is **diagnostic only**. It prioritizes system safety and does **not**:
- Run a release upgrade
- Install or remove packages
- Run `apt update`
- Edit APT sources
- Change Secure Boot settings or modify kernels
- Upload reports
- Require `sudo` for ordinary scans

Every report explicitly ends with: `No system changes were made.`

Target-release dependency simulation is not performed. The tool evaluates current package health, current-release `apt-get --simulate dist-upgrade`, storage, kernel, DKMS, Secure Boot, reboot state and static release policies.

## 💻 Supported Platforms

- **Primary diagnostic path**: Ubuntu 24.04 to Ubuntu 26.04 on `amd64` and `arm64`.
- **Secondary path**: Ubuntu 22.04 to Ubuntu 24.04.

*Pop!_OS 24.04 and other Ubuntu derivatives are detection-only and must not be treated as validated Ubuntu upgrade paths.*

## 🛠️ Build and Test

If you prefer to build from source, you can use CMake:

```bash
# Build and test debug preset
cmake --preset debug
cmake --build --preset debug
ctest --preset debug --output-on-failure

# Build and test release preset
cmake --preset release
cmake --build --preset release
ctest --preset release --output-on-failure
```

Optional presets are available for ASan and UBSan (`clang`, `asan`, `ubsan`, and `coverage`). Debian packaging metadata is included under `debian/`.

### Docker Validation

Docker validation uses exact Ubuntu 22.04, 24.04 and 26.04 tags:

```bash
bash docker/test-matrix.sh
```

> [!WARNING]
> Docker shares the host kernel and cannot validate native kernel, DKMS, Secure Boot, EFI, reboot, bootloader or full systemd behavior. Use a disposable native VM for those checks.

### Debian Artifacts

- Build Debian artifacts with `scripts/build-deb.sh`.
- Create series-specific source packages with `scripts/build-source-package.sh noble` or `scripts/build-source-package.sh resolute`.

The CLI version `0.1.0-beta.1` maps to Debian upstream version `0.1.0~beta1`; source packages append the Ubuntu-series suffix documented in [docs/launchpad-release.md](docs/launchpad-release.md). No script uploads artifacts.

## 🏗️ Architecture

The CLI depends on the application layer, which depends on ports and domain. Linux collectors and reporters are adapters. Rules are pure strategies over an immutable `SystemSnapshot`. Concrete production dependencies are assembled only in the composition root using constructor injection.
