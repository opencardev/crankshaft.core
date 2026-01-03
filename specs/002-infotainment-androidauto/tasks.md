# Tasks: Infotainment + Android Auto

**Input**: Design documents from `/specs/002-infotainment-androidauto/`
**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/

**Organization**: Tasks are grouped by user story to enable independent implementation and testing of each story.

## Format: `- [ ] [ID] [P?] [Story?] Description with file path`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (e.g., US1, US2, US3)
- Include exact file paths in descriptions

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Project initialization and basic structure

- [ ] T001 Create project structure per implementation plan in specs/002-infotainment-androidauto/
- [ ] T002 [P] Configure C++20/Qt6/CMake toolchain and validate build on WSL
- [ ] T003 [P] Add PipeWire dev packages and fallback PulseAudio support to build documentation
- [ ] T004 [P] Add SQLite dev dependency (libsqlite3-dev) to build docs and verify Qt SQL module linkage
- [ ] T005 [P] Setup JSON schema validation tooling for contracts/ (ajv-cli or similar)
- [ ] T006 Create sample extension directory structure under specs/002-infotainment-androidauto/examples/sample-extension/

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Core infrastructure that MUST be complete before ANY user story can be implemented

**⚠️ CRITICAL**: No user story work can begin until this phase is complete

- [x] T007 Implement WebSocket message validation helper in core/services/websocket/WebSocketServer.cpp (validateMessage, validateServiceCommand)
- [x] T008 [P] Implement structured error response helper (sendError) in core/services/websocket/WebSocketServer.cpp
- [x] T009 [P] Add SQLite-backed preferences store in core/services/preferences/PreferencesService.{h,cpp}
- [x] T010 [P] Add SQLite-backed session metadata store in core/services/session/SessionStore.{h,cpp}
- [x] T011 Add QtTest-based unit tests for WebSocket validation in tests/unit/test_websocket_validation.cpp
- [x] T012 [P] Add contract tests for ws-schema.json validation in tests/unit/test_contract_schemas.cpp
- [x] T013 [P] Add contract tests for extension-manifest.schema.json validation in tests/unit/test_contract_schemas.cpp
- [x] T014 Configure CTest to run unit and contract tests in CI pipeline (docs/testing-guide.md created; CMakeLists.txt updated)
- [x] T015 Document PipeWire/PulseAudio audio routing configuration in docs/audio-setup.md
- [x] T016 [P] Setup diagnostics REST endpoint skeleton (health, metrics) per contracts/openapi-diagnostics.yaml in core/services/diagnostics/DiagnosticsEndpoint.{h,cpp}

**Checkpoint**: Foundation ready - user story implementation can now begin in parallel

---

## Phase 3: User Story 1 - Boot to Home (Priority: P1) 🎯 MVP

**Goal**: Driver starts vehicle and Infotainment app launches within 10s showing Home screen with tiles (Media, Navigation, Phone, Settings)

**Independent Test**: Launch core+UI and measure cold-start time; verify Home tiles displayed and system time visible

### Implementation for User Story 1

- [x] T017 [P] [US1] Create Home screen QML component in ui/qml/screens/HomeScreen.qml with tile grid layout (Media, Navigation placeholder, Phone placeholder, Settings)
- [x] T018 [P] [US1] Create reusable Tile QML component in ui/qml/components/Tile.qml
- [x] T019 [P] [US1] Add system time display widget in ui/qml/components/SystemClock.qml
- [x] T020 [US1] Wire Home screen to main QML application entry point in ui/qml/main.qml
- [x] T021 [US1] Add i18n-ready strings for Home screen tiles (en-GB default) in ui/translations/en_GB.ts
- [x] T022 [US1] Implement cold-start performance benchmark script in tests/benchmarks/benchmark_cold_start.sh (target ≤10s)
- [x] T023 [US1] Add startup logging with timestamps in core/main.cpp and ui/main.cpp
- [x] T024 [US1] Update quickstart.md with instructions to run Home screen and benchmark

**Checkpoint**: At this point, User Story 1 should be fully functional - app boots to Home within 10s

---

## Phase 4: User Story 2 - Android Auto Connect (Priority: P1)

**Goal**: Driver connects Android phone via USB and Android Auto session begins within 15s showing AA UI surface

**Independent Test**: Connect compatible Android device; verify AA session negotiation completes and AA surface visible within 15s; audio routes to vehicle output

### Implementation for User Story 2

- [x] T025 [P] [US2] Verify external/aasdk supports wireless AA and document wireless mode flag in specs/002-infotainment-androidauto/aasdk-integration.md
- [x] T026 [P] [US2] Persist AndroidDevice entity (id, model, connection_type, paired) to SQLite in core/services/session/SessionStore.cpp
- [x] T027 [P] [US2] Persist Session entity (id, device_id, state, started_at, last_heartbeat) to SQLite in core/services/session/SessionStore.cpp
- [x] T028 [US2] Add AA connection state machine logic (negotiating→active→suspended→ended) in core/services/android_auto/RealAndroidAutoService.cpp
- [x] T029 [US2] Emit WebSocket events for AA lifecycle (android-auto/status/state-changed, connected, disconnected, error) in core/services/websocket/WebSocketServer.cpp
- [x] T030 [US2] Create QML AA projection surface component in ui/qml/screens/AndroidAutoScreen.qml
- [x] T031 [US2] Subscribe UI to AA events and display projection surface on connected event in ui/qml/main.qml
- [x] T032 [US2] Configure audio routing for AA media via PipeWire/PulseAudio in core/services/audio/AudioRouter.{h,cpp}
- [x] T033 [US2] Add AA connection performance benchmark in tests/benchmarks/benchmark_aa_connect.sh (target ≤15s)
- [x] T034 [US2] Add integration test for AA session lifecycle in tests/integration/test_aa_lifecycle.cpp
- [x] T035 [US2] Update quickstart.md with AA connection test instructions — Enhanced specs/002-infotainment-androidauto/quickstart.md with comprehensive device setup guide (USB debugging, wireless pairing), connection benchmarking (benchmark_aa_connect.sh, target ≤15s), session persistence testing (8 integration test cases), audio routing explanation (PipeWire/PulseAudio backends, MEDIA/GUIDANCE/SYSTEM roles, audio ducking with 40% reduction), and troubleshooting for USB/wireless connections and audio routing.

**Checkpoint**: At this point, User Stories 1 AND 2 should both work - boot to Home, connect AA device, see projection

---

## Phase 5: User Story 3 - Media Playback (Priority: P2)

**Goal**: Driver plays media via built-in player with responsive controls (play/pause/skip respond within 200ms)

**Independent Test**: Launch native media player from Home; load media file; verify play/pause/skip actions respond within 200ms

### Implementation for User Story 3

- [x] T036 [P] [US3] Create Media player QML screen in ui/qml/screens/MediaScreen.qml with playback controls
- [x] T037 [P] [US3] Implement MediaService in core/services/media/MediaService.{h,cpp} with play/pause/skip/seek commands
- [x] T038 [US3] Expose media control service commands via WebSocket (media.play, media.pause, media.skip) in core/services/websocket/WebSocketServer.cpp
- [x] T039 [US3] Add allowlisted media service commands to ws-schema.json command enum
- [x] T040 [US3] Wire Media screen controls to WebSocket service commands in ui/qml/screens/MediaScreen.qml
- [x] T041 [US3] Add performance test for media control latency (<200ms) in tests/benchmarks/benchmark_media_latency.sh
- [x] T042 [US3] Integrate MediaService with audio output routing in core/services/audio/AudioRouter.cpp
- [x] T043 [US3] Add i18n-ready media player strings in ui/translations/en_GB.ts

**Checkpoint**: Media playback functional with responsive controls; independent of AA

---

## Phase 6: User Story 4 - Settings Persistence (Priority: P2)

**Goal**: Driver toggles display preference (light/dark mode) in Settings; preference persists across restarts and applies immediately

**Independent Test**: Open Settings, toggle theme; restart app; verify theme persisted

### Implementation for User Story 4

- [x] T044 [P] [US4] Create Settings QML screen in ui/qml/screens/SettingsScreen.qml with theme toggle
- [x] T045 [P] [US4] Implement user preference read/write in core/services/preferences/PreferencesService.cpp (get/set for theme, locale)
- [x] T046 [US4] Expose settings service commands via WebSocket (settings.get, settings.set) in core/services/websocket/WebSocketServer.cpp
- [x] T047 [US4] Add allowlisted settings commands to ws-schema.json command enum
- [x] T048 [US4] Wire Settings screen controls to WebSocket service commands in ui/qml/screens/SettingsScreen.qml
- [x] T049 [US4] Implement light/dark theme QML styles in ui/qml/styles/Theme.qml
- [x] T050 [US4] Apply theme dynamically on preference change event in ui/qml/main.qml
- [x] T051 [US4] Add integration test for preference persistence across restarts in tests/integration/test_settings_persistence.cpp

**Checkpoint**: Settings functional with persistence; theme toggle works and survives restarts

---

## Phase 7: User Story 5 - Extension Update (Priority: P3)

**Goal**: Installer updates an extension; update installs and restarts extension without compromising core or other extensions

**Independent Test**: Run extension install/update via diagnostics API; verify extension restarts successfully and core remains stable

### Implementation for User Story 5

- [x] T052 [P] [US5] Implement extension manifest parser in core/services/extensions/ExtensionManager.cpp (parse extension-manifest.schema.json)
- [x] T053 [P] [US5] Implement extension lifecycle (start/stop/restart) with systemd-run or supervised child processes in core/services/extensions/ExtensionManager.cpp
- [x] T054 [US5] Add extension install/update REST endpoint (/extensions POST) per contracts/openapi-diagnostics.yaml in core/services/diagnostics/DiagnosticsEndpoint.cpp
- [x] T055 [US5] Add extension list REST endpoint (/extensions GET) in core/services/diagnostics/DiagnosticsEndpoint.cpp
- [x] T056 [US5] Implement extension permission enforcement (ui.tile, media.source, service.control, network, storage) in core/services/extensions/ExtensionManager.cpp
- [x] T057 [US5] Add cgroup resource limits for extension processes in core/services/extensions/ExtensionManager.cpp
- [x] T058 [US5] Create sample extension with manifest in specs/002-infotainment-androidauto/examples/sample-extension/manifest.json
- [x] T059 [US5] Create sample extension entrypoint script in specs/002-infotainment-androidauto/examples/sample-extension/run-sample-extension.sh
- [x] T060 [US5] Add integration test for extension install/restart without core crash in tests/integration/test_extension_lifecycle.cpp

**Checkpoint**: Extension system functional; sample extension can be installed/updated/restarted safely

---

## Phase 8: Polish & Cross-Cutting Concerns

**Purpose**: Improvements that affect multiple user stories

- [x] T061 [P] Add secure WebSocket (wss) support with certificate provisioning in core/services/websocket/WebSocketServer.cpp
- [x] T062 [P] Add comprehensive structured logging (JSON format) across all services in core/services/logging/Logger.cpp
- [x] T063 [P] Document contract versioning and migration policy in docs/contract-versioning.md
- [x] T064 [P] Add Design for Driving compliance checks (tap target sizes, contrast) in ui/qml/styles/
- [x] T065 [P] Add localization strings for additional locales (expand beyond en-GB) in ui/translations/
- [x] T066 [P] Implement driving-mode safety restrictions (block full keyboard, limit settings access) in ui/qml/main.qml
- [x] T067 [P] Add 24-hour soak test script in tests/integration/soak_test.sh (run core + 3 extensions for 24h)
- [x] T068 [P] Add memory/CPU profiling hooks and dashboard for performance monitoring in core/services/diagnostics/
- [x] T069 Run quickstart.md validation on clean WSL environment
- [x] T070 Update README.md with feature summary, installation, and quickstart link

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies - can start immediately
- **Foundational (Phase 2)**: Depends on Setup completion - BLOCKS all user stories
- **User Stories (Phase 3-7)**: All depend on Foundational phase completion
  - User Story 1 (Boot to Home) - P1 priority, MVP candidate
  - User Story 2 (Android Auto Connect) - P1 priority, MVP candidate, can proceed in parallel with US1 after foundational
  - User Story 3 (Media Playback) - P2 priority, can proceed after foundational
  - User Story 4 (Settings Persistence) - P2 priority, can proceed after foundational
  - User Story 5 (Extension Update) - P3 priority, can proceed after foundational
- **Polish (Phase 8)**: Depends on desired user stories being complete

### User Story Dependencies

- **User Story 1 (P1)**: Independent - only needs Foundational phase
- **User Story 2 (P1)**: Independent - only needs Foundational phase (can integrate with US1 for navigation flow)
- **User Story 3 (P2)**: Independent - only needs Foundational phase
- **User Story 4 (P2)**: Independent - only needs Foundational phase
- **User Story 5 (P3)**: Independent - only needs Foundational phase

### Within Each User Story

- Models/stores before services
- Services before UI components
- UI components before integration
- Benchmarks/tests can run in parallel with implementation once contracts defined

### Parallel Opportunities

- All Setup tasks marked [P] can run in parallel
- All Foundational tasks marked [P] can run in parallel (within Phase 2)
- Once Foundational phase completes, all 5 user stories can start in parallel (if team capacity allows)
- Within each story, tasks marked [P] can run in parallel
- Polish tasks marked [P] can all run in parallel

---

## Parallel Example: User Story 2 (Android Auto)

```bash
# Launch model/store tasks together:
Task T026: "Persist AndroidDevice entity to SQLite in core/services/session/SessionStore.cpp"
Task T027: "Persist Session entity to SQLite in core/services/session/SessionStore.cpp"

# Launch UI and audio config in parallel (different files):
Task T030: "Create QML AA projection surface component in ui/qml/screens/AndroidAutoScreen.qml"
Task T032: "Configure audio routing for AA media via PipeWire/PulseAudio in core/services/audio/AudioRouter.{h,cpp}"
```

---

## Implementation Strategy

### MVP First (User Stories 1 + 2 Only)

1. Complete Phase 1: Setup
2. Complete Phase 2: Foundational (CRITICAL - blocks all stories)
3. Complete Phase 3: User Story 1 (Boot to Home)
4. Complete Phase 4: User Story 2 (Android Auto Connect)
5. **STOP and VALIDATE**: Test US1+US2 independently; measure cold-start and AA connect times
6. Deploy/demo if ready

### Incremental Delivery

1. Complete Setup + Foundational → Foundation ready
2. Add User Story 1 → Test independently (cold start ≤10s) → Deploy/Demo (MVP tier 1!)
3. Add User Story 2 → Test independently (AA connect ≤15s) → Deploy/Demo (MVP tier 2!)
4. Add User Story 3 → Test independently (media latency <200ms) → Deploy/Demo
5. Add User Story 4 → Test independently (settings persist) → Deploy/Demo
6. Add User Story 5 → Test independently (extension update safe) → Deploy/Demo
7. Each story adds value without breaking previous stories

### Parallel Team Strategy

With multiple developers:

1. Team completes Setup + Foundational together
2. Once Foundational is done:
   - Developer A: User Story 1 (Boot to Home)
   - Developer B: User Story 2 (Android Auto Connect)
   - Developer C: User Story 3 (Media Playback)
   - Developer D: User Story 4 (Settings)
   - Developer E: User Story 5 (Extensions)
3. Stories complete and integrate independently

---

## Notes

- All tasks follow strict checklist format: `- [ ] [TaskID] [P?] [Story?] Description with file path`
- [P] tasks = different files, no dependencies, can run in parallel
- [Story] label maps task to specific user story for traceability (US1-US5)
- Each user story should be independently completable and testable
- Commit after each task or logical group
- Stop at any checkpoint to validate story independently
- Performance gates enforced via benchmark scripts (cold start, AA connect, media latency)
- Constitution compliance: tests required, structured logging, schema validation, i18n ready, secure WS
