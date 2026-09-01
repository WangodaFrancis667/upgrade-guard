# Beta Validation

Environment: Pop!_OS 24.04 development host plus disposable Ubuntu Docker images. Validation date: 2026-09-01.

| Test case | Expected result | Actual result | Status | Evidence |
| --- | --- | --- | --- | --- |
| Clean Ubuntu 24.04 fixture | Scan completes | Implemented fixture scan | Automated | `ctest --preset debug` |
| Held packages | Warning | Implemented fixture with held package | Automated | `ctest --preset debug` |
| Duplicate APT sources | Warning | Implemented fixture parser coverage | Automated | `ctest --preset debug` |
| Third-party source with credentials | Redacted JSON | Implemented reporter coverage | Automated | `ctest --preset debug` |
| Missing dkms | Not a crash | Implemented collector path | Automated | fake/host command contract |
| Missing mokutil | Unknown, not pass | Implemented collector path | Automated | fake/host command contract |
| Timed-out command | Typed timeout | Process runner supports timeout | Partial | no slow helper binary included |
| Oversized command output | Truncation flag | Fake runner tests truncation path | Partial | custom test harness |
| QEMU Secure Boot | Manual VM validation | Not executed | Not run | Requires VM setup |
| ARM64 | Native/VM validation | Not executed | Not run | Requires ARM64 environment |

| Ubuntu 22.04 Docker | Build, 4 CTest layers, JSON schema, Debian lifecycle, Lintian, autopkgtest | Passed | Automated | `artifacts/docker/22.04/` |
| Ubuntu 24.04 Docker | Build, 4 CTest layers, JSON schema, Debian lifecycle, Lintian, autopkgtest | Passed | Automated | `artifacts/docker/24.04/` |
| Ubuntu 26.04 Docker | Build, 4 CTest layers, JSON schema, Debian lifecycle, Lintian, autopkgtest | Passed | Automated | `artifacts/docker/26.04/` |
| Noble source package | Unsigned source-only artifacts | Passed | Automated | `artifacts/source/noble/` |
| Resolute source package | Unsigned source-only artifacts | Passed | Automated | `artifacts/source/resolute/` |

ASan and UBSan each passed all four native CTest layers. The ASan preset disables LeakSanitizer for the managed execution environment; AddressSanitizer instrumentation remains enabled. Docker proves userspace/package behavior only and is not native kernel, Secure Boot, EFI, reboot or systemd validation.

Target-release dependency simulation was not performed.
