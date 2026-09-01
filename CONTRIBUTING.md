# Contributing

Build with CMake presets and keep every hand-written C++ source or header below 400 physical lines. Add collectors as adapters that only gather facts, and add rules as pure strategies over `SystemSnapshot`.

Run:

```bash
cmake --preset debug
cmake --build --preset debug
ctest --preset debug --output-on-failure
```

Do not add scan-time mutation, telemetry, network calls or sudo usage.
