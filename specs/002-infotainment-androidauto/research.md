<!--
Research notes for Phase 0: Infotainment + Android Auto
Generated: 2025-12-30
-->

# Phase 0 — Research & Decisions

Each decision resolves a NEEDS CLARIFICATION item or captures best practices for dependencies/integrations. Format: Decision / Rationale / Alternatives considered.

## Decisions

1) C++ standard and toolchain
- Decision: Use C++20 (aligned with root `CMakeLists.txt`) with Qt6 toolchain and Ninja on WSL/Linux.
- Rationale: Matches repository configuration (`set(CMAKE_CXX_STANDARD 20)`), avoids divergence between core/UI and external/aasdk builds.
- Alternatives considered: C++17 (used in some external components) — rejected to keep parity with core build; modern features (std::span, concepts) expected in new code.

2) Audio stack for Android Auto and native playback
- Decision: Use PipeWire as primary audio server on Raspberry Pi OS; keep PulseAudio compatibility fallback for environments without stable PipeWire packages.
- Rationale: PipeWire offers lower-latency routing and flexible session management for simultaneous AA and native playback; Raspberry Pi OS supports PipeWire in current releases.
- Alternatives considered: PulseAudio-only (simpler but less flexible); raw ALSA (lower latency but harder to manage multiple clients); rejected due to routing complexity.

3) Persistence for preferences and session metadata
- Decision: Store preferences/device/session metadata in SQLite via Qt SQL module; cache lightweight state in memory with periodic flush.
- Rationale: SQLite provides durability and transactional safety for settings and AA session traces; Qt ships adapters and is already a dependency.
- Alternatives considered: JSON/QSettings files (simpler but weaker durability/locking); rejected due to risk of corruption and concurrency issues.

4) Test harness and coverage expectations
- Decision: Use CTest orchestration with QtTest-based unit/integration tests for core/UI, plus contract tests for JSON Schemas (ws + extension manifest) and OpenAPI diagnostics. Add performance/benchmark harness for startup and AA connect times (scripted under CTest where feasible).
- Rationale: Keeps alignment with existing CMake/CTest workflow; QtTest integrates with Qt signal/slot code; schema validation ensures protocol stability.
- Alternatives considered: Catch2 or GoogleTest integration — viable later but QtTest sufficient and already available with Qt kits.

5) WebSocket protocol and validation
- Decision: Adopt current protocol implemented in `core/services/websocket` and `ui` client: `subscribe`, `unsubscribe`, `publish`, `event`, `service_command`, `service_response`. Canonical schema lives at `contracts/ws-schema.json`; all inbound messages require `type`, `topic` (where applicable), and structured payload; reject unknown types and unauthorised commands.
- Rationale: Aligns with shipping code, avoids protocol churn, and enables schema-driven validation at boundaries.
- Alternatives considered: JSON-RPC 2.0 envelope — rejected to maintain compatibility with existing UI/extension clients.

6) Extension manifest and sandboxing
- Decision: Use JSON Schema (`contracts/extension-manifest.schema.json`) defining permissions (ui.tile, media.source, service_control, network, storage). Run extensions as separate OS processes via `systemd-run` or supervised child processes with per-extension user, cgroup resource limits, and seccomp/AppArmor profile where available.
- Rationale: Process isolation reduces blast radius; manifest-driven permissions make capability gating explicit.
- Alternatives considered: In-process plugins — rejected due to stability/security risk on embedded target.

7) Android Auto wireless support
- Decision: Use existing `external/aasdk` with RealAndroidAutoService for both USB and wireless; require Wi-Fi (AP/STA) and Bluetooth for pairing; document hardware-in-the-loop validation in `aasdk-integration.md`.
- Rationale: Matches current implementation, minimises vendor dependencies; wireless is a stated requirement.
- Alternatives considered: Deferring wireless to later — rejected per spec; alternate AA runtimes not adopted to stay aligned with existing code.

8) Security and transport hardening
- Decision: Deploy core WebSocket as secure WebSocket (wss) when certificates are provisioned; enforce input validation against `ws-schema.json`; restrict service commands to an allowlist; log and emit structured error responses.
- Rationale: Reduces attack surface for extensions/clients; meets constitution security clause.
- Alternatives considered: Plain ws without validation — rejected due to security risk.

## Next steps for Phase 1
- Update `contracts/ws-schema.json`, `extension-manifest.schema.json`, and `openapi-diagnostics.yaml` to reflect decisions above.
- Ensure test harness plan (QtTest + schema validation) is captured in tasks.
- Document PipeWire/PulseAudio packages and SQLite usage in quickstart/dev docs.
