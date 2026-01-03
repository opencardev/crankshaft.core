<!--
Sync Impact Report

- Version change: UNKNOWN -> 1.0.0
- Modified principles: (new) Code Quality & Maintainability; Test-First & Testing Standards; UX Consistency & Accessibility; Performance & Resource Constraints; Observability, Versioning & Change Management
- Added sections: Additional Constraints (Security, Licensing, Feature Flags); Development Workflow (Review Process, Quality Gates)
- Removed sections: none
- Templates requiring review/update:
	- .specify/templates/plan-template.md: ⚠ pending
	- .specify/templates/spec-template.md: ⚠ pending
	- .specify/templates/tasks-template.md: ⚠ pending
	- .specify/templates/checklist-template.md: ⚠ pending
	- .specify/templates/agent-file-template.md: ⚠ pending
- Follow-up TODOs:
	- TODO(RATIFICATION_DATE): Original ratification date unknown — please supply the project's original adoption date or confirm use of 2025-12-30.
-->

# Crankshaft Constitution

## Core Principles

### I. Code Quality & Maintainability
All code produced for Crankshaft MUST be readable, well-structured, and maintainable. Code MUST follow the project coding standards (file headers, naming, spacing, and style) and pass automated static analysis before merging. Public APIs and modules MUST include: clear purpose, minimal surface area, stable interfaces, and documentation sufficient for another engineer to use without additional explanation. Long-lived code MUST avoid hacks or undocumented workarounds; any technical debt MUST be recorded in an issue with a remediation plan.
Code duplication is prohibited.  
You must search for existing functions before creating a new one.  

Rationale: High-quality code reduces defects, eases onboarding, and lowers long-term maintenance cost.

Acceptance criteria:
- Linting and formatting pass in CI; code must compile cleanly where applicable.
- New public APIs include concise README entry and examples.
- Pull requests MUST include a short rationale for design choices when non-obvious.

### II. Test-First & Testing Standards (NON-NEGOTIABLE)
Tests are REQUIRED. For every production change that modifies behaviour, tests covering happy paths and key edge cases MUST be present. Unit tests are required for core logic; integration tests are required for cross-service behaviour and contracts. Test-first (TDD) is recommended for P1 work; at minimum, tests MUST be added alongside code changes and must pass in CI.

Rationale: Tests prevent regressions and codify expected behaviour.

Acceptance criteria:
- Unit tests for new features with deterministic outcomes.
- Integration tests for inter-service contracts and critical flows (startup, shutdown, message routing).
- CI gates require all tests to pass before merging.

### III. UX Consistency & Accessibility
The user experience MUST be consistent across screens, devices and modes. UI components MUST use shared design tokens, localization-ready strings (default `en-GB`), and support the light/dark theme toggle. Interactive elements exposed to drivers MUST follow 'Design for Driving' guidelines: minimal distraction, large tappable areas, predictable navigation flows. Accessibility features (clear contrast, scalable text, keyboard navigation where applicable) SHOULD be implemented for all user-facing screens.

Rationale: A consistent, accessible UI improves safety and reduces user error in driving contexts.

Acceptance criteria:
- Shared UI tokens and components are used for colours, spacing and typography.
- All user-visible strings are localised and default to `en-GB`.
- Key driving-mode screens meet contrast and hit-target size requirements.

### IV. Performance & Resource Constraints
Crankshaft MUST operate within the resource constraints of target hardware (Raspberry Pi 4 and comparable embedded platforms). Performance budgets MUST be defined for components and enforced in CI/performance tests. Critical goals: maintain responsive UI (target 60 fps for non-video UI, 30+ fps when rendering projection), ensure input latency under 50 ms for touch/tap events in projection scenarios, and keep steady-state memory footprint of core+UI within a defined budget (target <= 1.5 GB combined on supported hardware under normal load). Any feature that materially increases resource usage MUST include a performance regression test and a mitigation plan.

Rationale: The product must be reliable on constrained hardware used in vehicles.

Acceptance criteria:
- Performance tests for critical flows (startup time, AA connection, UI navigation) in CI or in a reproducible benchmark harness.
- Any PR that changes the runtime behaviour of services MUST document expected CPU/memory impact and include tests or measurements.

### V. Observability, Versioning & Change Management
All runtime components MUST emit structured logs and expose health and metrics endpoints where practical. Semantic versioning (MAJOR.MINOR.PATCH) MUST be used for public APIs and packages. Breaking changes to service contracts or public APIs require a MAJOR version change, a deprecation schedule, and migration guidance. Changes MUST be accompanied by release notes that document behaviour changes, migration steps, and rollbacks.

Rationale: Observability and clear versioning are essential for reliable operation, debugging, and safe upgrades.

Acceptance criteria:
- Structured logs (JSON) and metric hooks exist for services; CI validates log formatting where applicable.
- Release notes generated for every version bump that includes API/migration impact.
- Deprecation policy: minimum 2 release cycles of advance notice for breaking changes.

## Additional Constraints (Security, Licensing, Feature Flags)

- Security: All inputs from external sources MUST be validated or sanitized. Sensitive data (keys, tokens) MUST NOT be logged. Privileged operations MUST be gated by least-privilege controls and, where appropriate, sandboxing (e.g., extension isolation).
- Licensing & Feature Flags: The codebase MUST support feature gating and license-tier checks (Bronze/Silver/Gold/Platinum). Feature gates MUST be enforceable in core services (not only UI) and include deterministic rollout controls for staged exposure. Licensing validation MAY be server-assisted but MUST have an offline-safe fallback and audit logging for changes.
- Privacy: Telemetry and logs that leave the device MUST be anonymised and opt-in where required by local laws.

## Development Workflow, Review Process & Quality Gates

- Branching: Work SHOULD be developed in short-lived feature branches; PRs target `main` (or the release branch) and MUST reference an issue or spec.
- Code Review: Every PR MUST have at least one approving review from a maintainer. Reviews MUST verify tests, linting, and that the change follows the constitution principles. Architectural or security-sensitive changes MUST have a second technical review.
- CI Gates: PRs MUST pass linting, unit tests, integration tests (where applicable), and formatting checks before merge. Performance or resource-impacting changes MUST include performance validation artifacts.
- Pre-merge Checklist: include tests added, docs updated, migration notes (if any), and feature flag entries (if applicable).

## Governance

This constitution is the authoritative set of development principles for the Crankshaft project. Amendments follow the process below and are documented.

Amendment procedure:
- Propose: Create an Issue describing the amendment and rationale.
- Review: Obtain technical reviews and community review for at least 7 days (except emergency fixes).
- Ratify: A majority of active maintainers approve the amendment via PR to this file.
- Implement: Add any necessary migration scripts, template updates, or tests to accompany the change.

Versioning policy:
- MAJOR: Backwards-incompatible governance or API changes.
- MINOR: New principles, substantial additions, or new mandatory workflows.
- PATCH: Editorial changes, clarifications, or typo fixes.

Compliance and enforcement:
- All PRs are required to reference which constitution principles they affect and how compliance is ensured.
- CI and repository checks should automatically flag violations where feasible (linting, tests, license headers).

**Version**: 1.0.0 | **Ratified**: TODO(RATIFICATION_DATE): please supply original adoption date | **Last Amended**: 2025-12-30
