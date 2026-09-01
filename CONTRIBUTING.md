# 🛠️ Contributing

Build with CMake presets and keep every hand-written C++ source or header below 400 physical lines. Add collectors as adapters that only gather facts, and add rules as pure strategies over `SystemSnapshot`.

> [!CAUTION]
> Do not add scan-time mutation, telemetry, network calls or `sudo` usage.

## 💻 Build Instructions

Run the following commands to build and test:

```bash
# Debug preset
cmake --preset debug
cmake --build --preset debug
ctest --preset debug --output-on-failure

# ASan (AddressSanitizer) preset
cmake --preset asan
cmake --build --preset asan
ctest --preset asan

# UBSan (UndefinedBehaviorSanitizer) preset
cmake --preset ubsan
cmake --build --preset ubsan
ctest --preset ubsan

# Check release readiness
scripts/check-release.sh
```
