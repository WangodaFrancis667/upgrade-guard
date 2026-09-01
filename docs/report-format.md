# Report Format

JSON reports use schema version `1.0` with top-level fields:

`schema_version`, `tool_version`, `scan_id`, `started_at`, `completed_at`, `current_system`, `target_release`, `overall_status`, `summary`, `findings`, `limitations`, `privacy`.

`scan_id` and timestamps are intentionally nondeterministic. Rule ordering is stable.
