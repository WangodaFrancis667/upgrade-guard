# 📋 Release Requirements Matrix

This matrix records implementation state after the repository audit. 

> [!NOTE]
> “Verified” means exercised by the local CTest or build/package commands; environment-specific rows remain “implemented but unverified” until their external run succeeds.

| Area | Implementation | State |
| :--- | :--- | :--- |
| **Platform evidence and Ubuntu/derivative policy** | `os-release`, `uname`, container markers/cgroups; 22.04→24.04 and 24.04→26.04 only | ✅ implemented and verified |
| **Package cache and simulation** | narrow optional libapt-pkg adapter plus bounded current-release `apt-get` simulation; no `apt update` | ⚠️ implemented and verified without local libapt-pkg; packaged adapter unverified |
| **dpkg, holds and sources** | dpkg audit, apt-mark holds, list/deb822 parsing, duplicates, disabled and third-party classification | ✅ implemented and verified |
| **Storage** | `statvfs` totals/free space, device-based separate-mount discovery and optional EFI | ✅ implemented and verified |
| **Kernel, DKMS and Secure Boot** | kernel packages, fallback, headers/initramfs, DKMS states, `mokutil` and container limitation | ⚠️ implemented and verified with fixtures; native firmware unverified |
| **Reboot** | marker, package list and read errors | ✅ implemented and verified |
| **18 readiness rules** | exactly one registration per required ID; unknown evidence does not become ready | ✅ implemented and verified |
| **CLI** | scan, export, explain, list-checks, version/help, options, overwrite protection and exit codes | ✅ implemented and verified |
| **Reports** | deterministic text/JSON, schema v1, counts, timing, scan ID, limitations, privacy and default redaction | ✅ implemented and verified |
| **Process execution** | fixed allowlist, execve argument vector, sanitized locale/PATH, separate pipes, timeout/reap and per-stream bounds | ✅ implemented and verified |
| **Native tests and presets** | unit, contract, fixture integration, CLI; GCC, Clang, ASan, UBSan and coverage presets | ⚠️ implemented; availability-dependent gates unverified |
| **Docker matrix** | exact 22.04/24.04/26.04 builds, CTest, JSON, package lifecycle, Lintian and autopkgtest with per-release artifacts | ⚠️ implemented but unverified |
| **Debian/Launchpad packaging** | quilt source, binary/source scripts, noble/resolute versions, install smoke and manual-upload docs | ⚠️ implemented but unverified |
