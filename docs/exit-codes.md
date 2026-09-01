# Exit codes

| Code | Meaning |
| ---: | --- |
| 0 | Ready |
| 1 | Ready with warnings |
| 2 | Blocked by a readiness finding |
| 3 | Incomplete evidence or operational error |
| 64 | Invalid command-line usage |

Unknown essential evidence cannot yield exit code 0. Export-file conflicts and write failures are operational errors and return 3.
