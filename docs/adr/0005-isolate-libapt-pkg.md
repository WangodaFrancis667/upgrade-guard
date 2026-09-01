# 0005 Isolate libapt-pkg

APT integration belongs behind package-health adapters. APT types must not leak into domain, application, CLI or reporting code.
