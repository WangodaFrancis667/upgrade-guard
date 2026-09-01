# Contributing

Build with CMake presets and keep every hand-written C++ source or header below 400 physical lines. Add collectors as adapters that only gather facts, and add rules as pure strategies over `SystemSnapshot`.

Run:

```bash
cmake --preset debug
cmake --build --preset debug
ctest --preset debug --output-on-failure
cmake --preset asan && cmake --build --preset asan && ctest --preset asan
cmake --preset ubsan && cmake --build --preset ubsan && ctest --preset ubsan
scripts/check-release.sh
```

Do not add scan-time mutation, telemetry, network calls or sudo usage.
