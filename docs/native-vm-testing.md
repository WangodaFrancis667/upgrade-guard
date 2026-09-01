# 🖥️ Native VM Validation

Docker is useful for compiler, CLI and Debian package portability but shares the host kernel. 

Before a stable release, use disposable Ubuntu 22.04, 24.04 and 26.04 VMs, including `amd64` and `arm64` where available, to test real kernel inventory, fallback kernels, header/initramfs alignment, DKMS states, Secure Boot enabled/disabled/unknown, EFI storage, reboot markers, bootloaders and systemd behavior.

> [!CAUTION]
> Take a VM snapshot first. Do not mount host `/etc`, `/boot`, APT or dpkg state into the guest. 

A normal Upgrade Guard scan remains read-only; package install, upgrade, removal and reinstall tests apply only to the packaged tool inside the guest. Record image checksums, architecture, firmware mode and exact results in `docs/beta-validation.md`.
