# Feature Specification: [FEATURE NAME]
<!--
Feature spec created by speckit: Build an Infotainment application for automotive use.
Feature Branch: `002-infotainment-androidauto`
Created: 2025-12-30
Status: Draft
-->

# Build Infotainment application with Android Auto support

## Summary

Provide a production-ready Infotainment application for automotive use that supports core vehicle infotainment features (media, navigation, phone, settings) and includes Android Auto integration so users can connect and use their Android devices safely while driving.

## Background and Goals

Modern vehicles require a cohesive infotainment experience that exposes media playback, navigation, phone integration and vehicle settings. The project should prioritise safety, reliability, and low resource usage so it runs well on target hardware (e.g., Raspberry Pi-class platforms). Android Auto must be supported to allow phone integration via the standard user experience.

## Actors

- Primary actor: Vehicle driver (end user)
- Secondary actor: Passenger (end user)
- Admin / Installer (system integrator who installs/updates the software)

## Scope

- In scope:
  - Core UI for an in-vehicle infotainment app (home screen, media, navigation placeholder, phone/dialer, settings).
  - Android Auto integration enabling the Android Auto experience when a compatible device is connected (wired or wireless where supported).
  - Extension framework hooks so additional features (radio, Bluetooth, media players) can be plugged in.
  - Basic localisation support (default locale en-GB) and accessibility considerations.
  - Security sandboxing to protect core from extensions.

- Out of scope:
  - Deep navigation provider integrations (map data procurement and licensing) beyond a navigation placeholder and integration points.
  - Full app store infrastructure for extensions (the extension store can be planned later).

## User Scenarios & Testing

### P1 - Boot to Home (Driver)

Driver starts vehicle — the Infotainment app launches within 10s and shows Home screen with primary tiles (Media, Navigation, Phone, Settings).

Acceptance tests:
- App launches on boot or when started manually within 10s (measured cold start on target device).
- Home screen displays tiles and system time.

### P1 - Android Auto Connect (Driver)

Driver connects Android phone via USB — Android Auto session begins and Android Auto UI is available on the display; user can open compatible apps (maps, media, messaging) through the Android Auto interface.

Acceptance tests:
- When a compatible Android device is connected, the Android Auto session negotiates and shows an Android Auto surface within 15s.
- Basic audio routing switches to Android Auto media playback.

### P2 - Media Playback (Driver)

Driver plays media via built-in player — media controls available on UI and via steering wheel controls (if present).

Acceptance tests:
- Play/pause/skip respond within 200ms of UI action.

### P2 - Settings Persistence (Driver)

Driver opens Settings and toggles a display preference (light/dark mode) — preference is persisted and applied immediately.

Acceptance tests:
- Preference persists across restarts.

### P3 - Extension Update (Installer)

Installer updates an extension — extension update installs and restarts the extension without compromising core or other extensions.

Acceptance tests:
- Extension update procedure completes without crashing core app and returns success status.

### Edge Cases

- Unsupported Android device connects: system shows clear message and falls back to native UI.
- Multiple Android devices connected: system uses last active device or prompts user to choose.
- Low-memory scenario: extensions with lower priority are suspended to preserve core functionality.

## Functional Requirements (testable)

FR-1: App Startup
- The Infotainment application must start and present the Home screen within 10 seconds on target hardware from a cold boot.

FR-2: Home Screen
- The Home screen shows at minimum: Media, Navigation (placeholder), Phone (placeholder), Settings tiles and current time. Note: Phone and Navigation implementations are out-of-scope for initial delivery; tiles serve as UI placeholders for future integration.

FR-3: Android Auto: Session Establishment
- When a compatible Android device is connected, the system must establish an Android Auto session and present the Android Auto surface within 15 seconds.

FR-4: Android Auto: Audio Routing
- Android Auto (AA) media audio must be routed through the vehicle audio output and controllable through built-in media controls (on-screen UI controls; steering wheel/physical button integration is implementation-defined).

FR-5: Extension API
- Provide a documented extension manifest and API surface so third-party extensions can register UI tiles, media sources, and background services. Extension actions must be permissioned and run in a sandbox.

FR-6: Settings Persistence
- User preferences (e.g., display mode, language) must persist across restarts and be stored reliably.

FR-7: Safety Mode
- When the vehicle is in drive (or an equivalent motion/lock state is active), restrict interaction with high-attention features (e.g., full keyboard input blocked, detailed settings access limited). Exact gating must be configurable by policy. Note: Vehicle state detection mechanism (OBD-II/CAN signal, GPS speed threshold, or manual toggle) is implementation-defined; initial delivery includes UI-level restrictions only.

FR-8: Localisation
- UI strings must be localisable with default locale en-GB and support adding further locales.

FR-9: Security
- Extensions must be sandboxed; an extension failure must not crash the core app.

FR-10: Diagnostics and Logging
- Core must include structured logs and a diagnostics endpoint for status and health checks accessible to installers.

## Non-Functional Requirements

- Performance: Cold start <= 10s; Visible UI interactions respond within 200ms.
- Resilience: Core must continue running if any single extension crashes.
- Resource use: Reasonable CPU and memory for Raspberry Pi 4-class hardware. (Quantitative targets to be added during planning.)

## Success Criteria (measurable)

- Users can boot and reach Home screen within 10 seconds (measured on target device) in 90% of cold-start tests.
- Android Auto sessions start and present usable UI within 15 seconds for 95% of tested Android devices.
- Core remains running with 0 crashes during a standard 24-hour soak test with at least 3 installed extensions.
- 95% of UI actions (play/pause, navigation open, settings change) complete within 200ms.
- Extension install/update operations complete successfully in >= 98% of attempts during test runs.

## Key Entities

- User: driver or passenger interacting with UI.
- Android Device: phone providing Android Auto session.
- Extension: packaged component with manifest exposing features (UI tile, service, media source).
- Session: Android Auto connection session state and lifecycle.

## Assumptions

- Target hardware will be Raspberry Pi 4-class or equivalent (arm64), running a Linux distribution compatible with project build tools.
- Android Auto support will be integrated via a compatible Android Auto runtime or SDK; the specific implementation choice will be decided during planning.
- Map/data providers and licensing for turn-by-turn navigation are out-of-scope for initial delivery.

## Dependencies

- Android Auto runtime or SDK and its build/runtime dependencies.
- Media playback backends and audio routing libraries present on the platform.

## Risks

- Android Auto certification/licensing specifics may impose additional requirements for shipping; this spec does not cover certification compliance.
- Hardware/performance constraints on Raspberry Pi-class devices may require feature trade-offs.

## Out-of-scope / Future

- Full extension store and management UI (deferred to future iteration).
- Built-in navigation provider with map tiles and routing — only integration points provided.

## Acceptance Criteria

- All functional requirements above have automated or manual acceptance tests defined in the Test Plan.
- Android Auto integration demonstrated with at least two Android device models.
- Spec passes the spec quality checklist in `checklists/requirements.md`.

## Next Steps

1. Move to `/speckit.clarify` if any of the assumptions or in-scope items need further clarification.
2. Proceed to planning (`/speckit.plan`) to break down implementation tasks.

---

Spec status: Draft
