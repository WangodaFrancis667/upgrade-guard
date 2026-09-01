# Rule Authoring

Rules implement `IReadinessRule` and evaluate only `SystemSnapshot`. They must not read files, run commands or format reports. Each finding needs a stable ID, status, severity, title, explanation, evidence, recommendation, confidence and collector completeness flag.

Unknown evidence must remain `unknown` or `error`; it must never become `passed`.
