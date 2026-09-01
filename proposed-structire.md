upgrade-guard/
├── CMakeLists.txt
├── CMakePresets.json
├── LICENSE
├── README.md
├── SECURITY.md
├── CONTRIBUTING.md
├── CODE_OF_CONDUCT.md
├── CHANGELOG.md
│
├── cmake/
│   ├── CompilerWarnings.cmake
│   ├── Sanitizers.cmake
│   ├── StaticAnalysis.cmake
│   └── Testing.cmake
│
├── apps/
│   └── cli/
│       ├── CMakeLists.txt
│       ├── include/
│       │   └── upgrade_guard/cli/
│       │       ├── CliApplication.hpp
│       │       └── ExitCode.hpp
│       └── src/
│           ├── commands/
│           │   ├── ScanCommand.cpp
│           │   ├── ExplainCommand.cpp
│           │   ├── ExportCommand.cpp
│           │   └── ListChecksCommand.cpp
│           ├── ConsolePresenter.cpp
│           ├── CompositionRoot.cpp
│           └── main.cpp
│
├── core/
│   ├── domain/
│   │   ├── CMakeLists.txt
│   │   ├── include/upgrade_guard/domain/
│   │   │   ├── Finding.hpp
│   │   │   ├── FindingId.hpp
│   │   │   ├── Severity.hpp
│   │   │   ├── Evidence.hpp
│   │   │   ├── Confidence.hpp
│   │   │   ├── ReadinessStatus.hpp
│   │   │   ├── RemediationAdvice.hpp
│   │   │   ├── SystemSnapshot.hpp
│   │   │   ├── TargetRelease.hpp
│   │   │   └── Result.hpp
│   │   └── src/
│   │
│   ├── contracts/
│   │   ├── CMakeLists.txt
│   │   └── include/upgrade_guard/contracts/
│   │       ├── ISystemCollector.hpp
│   │       ├── IReadinessRule.hpp
│   │       ├── IProcessRunner.hpp
│   │       ├── IFileSystem.hpp
│   │       ├── IClock.hpp
│   │       ├── IReporter.hpp
│   │       └── IScanRepository.hpp
│   │
│   └── application/
│       ├── CMakeLists.txt
│       ├── include/upgrade_guard/application/
│       │   ├── ScanSystem.hpp
│       │   ├── ExplainFinding.hpp
│       │   ├── ExportReport.hpp
│       │   ├── SnapshotBuilder.hpp
│       │   └── ReadinessEvaluator.hpp
│       └── src/
│
├── modules/
│   ├── platform/
│   │   ├── domain/
│   │   ├── rules/
│   │   └── adapters/
│   │       ├── OsReleaseAdapter.cpp
│   │       ├── ArchitectureAdapter.cpp
│   │       └── UbuntuReleaseAdapter.cpp
│   │
│   ├── package_health/
│   │   ├── domain/
│   │   ├── rules/
│   │   │   ├── BrokenPackagesRule.cpp
│   │   │   ├── HeldPackagesRule.cpp
│   │   │   ├── ThirdPartySourcesRule.cpp
│   │   │   ├── DuplicateSourcesRule.cpp
│   │   │   └── UpgradeSimulationRule.cpp
│   │   └── adapters/
│   │       ├── AptCacheAdapter.cpp
│   │       ├── AptSourcesAdapter.cpp
│   │       ├── AptSimulationAdapter.cpp
│   │       └── DpkgAuditAdapter.cpp
│   │
│   ├── storage_health/
│   │   ├── domain/
│   │   ├── rules/
│   │   │   ├── RootSpaceRule.cpp
│   │   │   ├── BootSpaceRule.cpp
│   │   │   └── EfiSpaceRule.cpp
│   │   └── adapters/
│   │       └── PosixDiskSpaceAdapter.cpp
│   │
│   ├── kernel_health/
│   │   ├── domain/
│   │   ├── rules/
│   │   │   ├── FallbackKernelRule.cpp
│   │   │   ├── DkmsHealthRule.cpp
│   │   │   └── InitramfsHealthRule.cpp
│   │   └── adapters/
│   │       ├── KernelAdapter.cpp
│   │       ├── DkmsAdapter.cpp
│   │       └── InitramfsAdapter.cpp
│   │
│   ├── security_health/
│   │   ├── domain/
│   │   ├── rules/
│   │   │   └── SecureBootDkmsRule.cpp
│   │   └── adapters/
│   │       └── MokutilAdapter.cpp
│   │
│   ├── reboot_health/
│   │   ├── rules/
│   │   │   └── PendingRebootRule.cpp
│   │   └── adapters/
│   │       └── RebootRequiredAdapter.cpp
│   │
│   └── reporting/
│       ├── TextReporter.cpp
│       ├── JsonReporter.cpp
│       └── RedactingReporter.cpp
│
├── infrastructure/
│   ├── process/
│   │   ├── PosixProcessRunner.cpp
│   │   └── ProcessResult.hpp
│   ├── filesystem/
│   │   └── PosixFileSystem.cpp
│   ├── time/
│   │   └── SystemClock.cpp
│   └── logging/
│       └── Logger.cpp
│
├── schemas/
│   └── report-v1.schema.json
│
├── tests/
│   ├── unit/
│   ├── contract/
│   ├── integration/
│   ├── system/
│   ├── fixtures/
│   │   ├── ubuntu-22.04/
│   │   ├── ubuntu-24.04/
│   │   ├── ubuntu-26.04/
│   │   └── pop-os-24.04/
│   ├── golden-reports/
│   └── fakes/
│       ├── FakeProcessRunner.hpp
│       ├── FakeFileSystem.hpp
│       ├── FakeClock.hpp
│       └── SnapshotFactory.hpp
│
├── docs/
│   ├── architecture.md
│   ├── threat-model.md
│   ├── privacy.md
│   ├── supported-platforms.md
│   ├── rule-authoring.md
│   ├── report-format.md
│   └── adr/
│       ├── 0001-use-cpp20.md
│       ├── 0002-modular-monolith.md
│       ├── 0003-read-only-first-release.md
│       ├── 0004-manual-dependency-injection.md
│       └── 0005-isolate-libapt-pkg.md
│
├── man/
│   └── upgrade-guard.1
│
├── debian/
│   ├── changelog
│   ├── control
│   ├── copyright
│   ├── rules
│   ├── install
│   ├── manpages
│   ├── source/format
│   └── tests/
│       ├── control
│       └── smoke
│
└── .github/
    ├── workflows/
    │   ├── build.yml
    │   ├── test.yml
    │   ├── sanitizers.yml
    │   └── package.yml
    └── ISSUE_TEMPLATE/