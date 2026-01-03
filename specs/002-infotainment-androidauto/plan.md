# Implementation Plan: Infotainment + Android Auto

**Branch**: `002-infotainment-androidauto` | **Date**: 2025-12-30 | **Spec**: `/specs/002-infotainment-androidauto/spec.md`
**Input**: Feature specification from `/specs/002-infotainment-androidauto/spec.md`

## Summary

Deliver a production-ready infotainment application with Android Auto (wired + wireless) on Raspberry Pi-class hardware. Reuse existing C++/Qt6 core and UI with WebSocket-based messaging between core, UI and extensions; integrate AASDK for Android Auto transport; enforce extension isolation (separate processes) with manifest-defined permissions. Performance gates: cold start ≤ 10s, Android Auto surface ≤ 15s, UI interactions ≤ 200 ms.

## Technical Context

**Language/Version**: C++ (C++20 standard, resolved in research.md) with Qt6 for core/UI; shell/Python for tooling.
**Primary Dependencies**: Qt6 (QML, QWebSocket, multimedia, Qt SQL), AASDK (Android Auto), CMake/Ninja toolchain, PipeWire (primary) with PulseAudio fallback for audio (resolved in research.md), systemd integration for services.
**Storage**: SQLite via Qt SQL module for preferences and session metadata (resolved in research.md).
**Testing**: CTest orchestration with QtTest for unit/integration tests plus JSON schema validation for contracts (resolved in research.md).
**Target Platform**: Raspberry Pi OS (arm64/armhf) and Linux dev hosts (WSL/Ubuntu); Qt6 EGLFS/VNC display backends.
**Project Type**: Embedded multi-process (core daemon, UI client, extension processes) with WebSocket/event-bus messaging.
**Performance Goals**: Cold start ≤10s; Android Auto connect ≤15s; UI actions ≤200 ms; steady-state memory target ≤1.5 GB core+UI; touch latency <50 ms in projection mode.
**Constraints**: Driver-safe UX (Design for Driving), en-GB default locale with i18n ready, sandboxed extensions, secure WebSocket (wss) and input validation, offline-capable operation.
**Scale/Scope**: Single-vehicle head unit with multiple extensions; extension store deferred; limited concurrent clients (UI + extensions + diagnostics).

## Constitution Check
*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

- Code Quality & Maintainability: Use project headers, Qt/CMake style, documented public APIs; plan includes lint/format and README updates for new contracts.
- Test-First & Testing Standards: Require unit/integration tests for WS validation/auth, service commands, and Android Auto lifecycle; add contract tests for schemas; CI must run CTest and schema validation.
- UX Consistency & Accessibility: Light/dark toggle, en-GB default, shared tokens; Android Auto and core UI follow Design for Driving (large tap targets, limited distraction); ensure localisation-ready strings in extensions.
- Performance & Resource Constraints: Enforce budgets (startup, AA connect, memory) with benchmarks/measurements; avoid regressions in embedded targets.
- Observability, Versioning & Change Management: Structured logging, diagnostics endpoint (OpenAPI), semantic versioning for contracts; document breaking changes and migration notes.

GATE status: No violations noted. Phase 0 research resolved prior unknowns (C++20 toolchain, PipeWire primary audio with PulseAudio fallback, SQLite persistence, QtTest/CTest harness); continue to enforce schema validation, allowlisted service commands, and wss hardening in implementation.

## Project Structure

### Documentation (this feature)

```text
specs/002-infotainment-androidauto/
├── plan.md
├── research.md
├── data-model.md
├── quickstart.md
├── contracts/
│   ├── ws-schema.json
│   ├── extension-manifest.schema.json
│   └── openapi-diagnostics.yaml
├── research/ (if expanded for sub-topics)
├── aasdk-integration.md
└── tasks.md (Phase 2)
```

### Source Code (repository root)

```text
core/                 # C++ core services (WebSocketServer, EventBus, AndroidAuto service)
ui/                   # Qt/QML UI client (WebSocket client)
external/aasdk/       # Android Auto SDK integration
scripts/, tools/      # build, lint, formatting scripts (WSL/Linux)
docs/                 # project documentation
tests/                # unit/integration tests (CTests)
packaging/, docker/   # packaging and deployment assets
```

**Structure Decision**: Existing embedded multi-process layout retained (core daemon + UI + extensions communicating over WebSocket/EventBus). Documentation and contracts live under `specs/002-infotainment-androidauto/`; runtime code changes stay within `core/`, `ui/`, `external/aasdk/`, and extension samples under `specs/.../examples/`.

## Complexity Tracking

| Violation | Why Needed | Simpler Alternative Rejected Because |
|-----------|------------|---------------------------------------|
| None | N/A | N/A |
