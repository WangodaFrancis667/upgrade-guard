# Docker validation

Run `bash docker/test-matrix.sh` to build and test Ubuntu 22.04, 24.04 and 26.04 images. Logs and packages are written under `artifacts/docker/<release>/`.

The compose services mount only the source tree read-only and `artifacts/` read-write. They do not use privileged mode, host PID/network namespaces, the Docker socket, or host package/boot directories. Package install, removal and reinstall happen only in the disposable container.

Docker shares the host kernel. These runs are not proof of kernel isolation, DKMS module builds, Secure Boot, EFI, bootloader, reboot or full systemd behavior. Validate those cases with fixtures and native QEMU/VM tests.
