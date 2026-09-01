# Architecture

Upgrade Guard is a modular monolith with hexagonal boundaries:

- CLI parses commands and selects reporters.
- Application coordinates collectors and rules through ports.
- Domain owns immutable facts, findings and reports.
- Adapters gather read-only Linux evidence.
- Rules interpret snapshots without files, commands, APT headers or report formatting.

CMake targets mirror these boundaries. `upgrade_guard_application` links only to ports and domain. Module libraries link to ports. The CLI target is the composition root and constructs concrete dependencies.

Dependency injection is manual constructor injection. There is no container, singleton or service locator.

`libapt-pkg` is isolated in `AptCacheAdapter.cpp`; its types do not cross the package-health module. Builds with the development package use it for cache, broken-count and upgradable-package evidence. A filesystem/current-release simulation fallback keeps developer builds possible when the headers are unavailable.
