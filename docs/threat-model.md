# Threat Model

The main risks are accidental mutation, command injection, sensitive data disclosure and false readiness claims. The process runner uses fixed executable names, explicit argument vectors, no shell and bounded output. Reports redact credentials embedded in repository URLs and home-directory paths.

The scanner never runs `apt update`, edits sources, installs packages, removes packages, modifies kernels, changes Secure Boot or uploads data.
