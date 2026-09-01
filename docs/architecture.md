# Architecture

Upgrade Guard is a modular monolith with hexagonal boundaries:

- CLI parses commands and selects reporters.
- Application coordinates collectors and rules through ports.
- Domain owns immutable facts, findings and reports.
- Adapters gather read-only Linux evidence.
- Rules interpret snapshots without files, commands, APT headers or report formatting.

CMake targets mirror these boundaries. `upgrade_guard_application` links only to ports and domain. Module libraries link to ports. The CLI target is the composition root and constructs concrete dependencies.

Dependency injection is manual constructor injection. There is no container, singleton or service locator.

`libapt-pkg` is intentionally isolated as a future narrow package-health adapter boundary. This beta build keeps APT integration process/file based when development headers are unavailable, and the Debian metadata declares `libapt-pkg-dev` for package builds that add the library adapter.
