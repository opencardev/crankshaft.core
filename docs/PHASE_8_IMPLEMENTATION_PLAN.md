# Phase 8: Polish & Cross-Cutting Concerns - Implementation Plan

**Status**: Starting Phase 8 (Tasks T061-T070)  
**Previous Phases**: 1-7 Complete (87.5% of project)  
**Total Project**: 7/8 phases complete (this phase completes 100%)  
**Date Started**: January 3, 2026

---

## Phase 8 Overview

**Purpose**: Add production-grade polish and cross-cutting concerns that improve security, reliability, maintainability, and compliance across all user stories.

**Priority**: P1 (Recommended for production release)

**Expected Duration**: 3-5 days for a single developer

**Test Impact**: Phase 8 tasks build on Phases 1-7 (55/60 tests already passing)

---

## Tasks Breakdown (T061-T070)

### Security & Communication

#### T061: Secure WebSocket (wss) Support [P]
**Goal**: Enable encrypted WebSocket connections for production deployment

**Deliverables**:
- [ ] Self-signed certificate generation script
- [ ] wss:// endpoint alongside ws://
- [ ] Certificate path configuration
- [ ] Client certificate validation
- [ ] Upgrade ws:// to wss:// on production builds

**Files to Create/Modify**:
- `core/services/websocket/WebSocketServer.cpp` - Add TLS/SSL support
- `core/services/websocket/WebSocketServer.h` - Add certificate configuration
- `scripts/generate-ssl-certs.sh` - Certificate generation utility
- `config/crankshaft.conf` - wss configuration settings

**Dependencies**: None (Phase 7 independent)

**Estimated Effort**: 4-6 hours

---

#### T062: Structured JSON Logging [P]
**Goal**: Implement comprehensive structured logging for debugging and monitoring

**Deliverables**:
- [ ] JSON log format with timestamp, level, component, context
- [ ] Log rotation and archiving
- [ ] Log severity filtering (DEBUG, INFO, WARN, ERROR, FATAL)
- [ ] Contextual logging (request IDs, extension IDs, session IDs)
- [ ] Structured logger service available to all components

**Files to Create/Modify**:
- `core/services/logging/Logger.cpp` - JSON log formatter
- `core/services/logging/Logger.h` - Logger interface
- `core/services/logging/LoggerConfig.h` - Configuration constants
- `core/main.cpp` - Initialize logging on startup
- Update all services to use structured logging

**Dependencies**: None (Phase 7 independent)

**Estimated Effort**: 6-8 hours

---

### Documentation & Maintenance

#### T063: Contract Versioning Policy [P]
**Goal**: Document API/contract evolution strategy for long-term maintainability

**Deliverables**:
- [ ] Version numbering scheme (semantic versioning)
- [ ] Backward compatibility guarantees
- [ ] Deprecation policy
- [ ] Migration guide examples
- [ ] Contract changelog template

**Files to Create**:
- `docs/contract-versioning.md` - Complete versioning policy
- Example: Version upgrade migrations

**Dependencies**: None (documentation only)

**Estimated Effort**: 2-3 hours

---

### UI/UX Compliance

#### T064: Design for Driving Compliance [P]
**Goal**: Ensure UI meets automotive safety standards (Google Design for Driving)

**Deliverables**:
- [ ] Tap target size validation (48dp minimum per Android Auto specs)
- [ ] Text contrast checking (WCAG AA minimum)
- [ ] Readability at glance (max 2-3 seconds per screen)
- [ ] Safe color schemes (colorblind-friendly)
- [ ] Haptic feedback for long-press actions

**Files to Create/Modify**:
- `ui/qml/styles/Theme.qml` - Add accessibility constants
- `ui/qml/components/Tile.qml` - Verify tap targets
- `ui/qml/components/Button.qml` (new) - Safe button component
- `docs/design-for-driving.md` - Compliance checklist

**Dependencies**: UI components from Phases 3-6

**Estimated Effort**: 4-5 hours

---

#### T065: Localization Expansion [P]
**Goal**: Add support for multiple languages beyond en-GB default

**Deliverables**:
- [ ] en-US translations (American English)
- [ ] de-DE translations (German)
- [ ] fr-FR translations (French)
- [ ] es-ES translations (Spanish)
- [ ] Locale selection in Settings screen
- [ ] Dynamic locale switching

**Files to Create/Modify**:
- `ui/translations/en_US.ts` (new)
- `ui/translations/de_DE.ts` (new)
- `ui/translations/fr_FR.ts` (new)
- `ui/translations/es_ES.ts` (new)
- `ui/qml/screens/SettingsScreen.qml` - Add locale selector
- `core/services/preferences/PreferencesService.cpp` - Store locale preference

**Dependencies**: i18n framework from Phase 1, Settings UI from Phase 6

**Estimated Effort**: 3-4 hours

---

### Safety & Restrictions

#### T066: Driving Mode Safety Restrictions [P]
**Goal**: Prevent driver distraction with safety restrictions while driving

**Deliverables**:
- [ ] Driving mode detection (speed-based or manual toggle)
- [ ] Block full keyboard input in driving mode
- [ ] Limit settings access to safe options only
- [ ] Large, easy-to-hit controls in driving mode
- [ ] Haptic feedback for confirmations
- [ ] Speed-based UI restrictions

**Files to Create/Modify**:
- `core/services/driving/DrivingModeService.h` (new)
- `core/services/driving/DrivingModeService.cpp` (new)
- `ui/qml/main.qml` - Integrate driving mode restrictions
- `ui/qml/screens/SettingsScreen.qml` - Hide unsafe settings in driving mode
- `ui/qml/styles/Theme.qml` - Driving mode style overrides

**Dependencies**: Settings from Phase 6, core service framework from Phase 2

**Estimated Effort**: 5-6 hours

---

### Testing & Quality Assurance

#### T067: 24-Hour Soak Test [P]
**Goal**: Verify system stability under continuous operation with multiple extensions

**Deliverables**:
- [ ] Soak test script running core + 3 sample extensions
- [ ] Memory usage tracking (no leaks)
- [ ] CPU usage monitoring
- [ ] Process restart recovery
- [ ] Log analysis for errors
- [ ] Automated test report generation

**Files to Create**:
- `tests/integration/soak_test.sh` - Main test script
- `tests/integration/soak_test_extensions/` - 3 test extensions
- `tests/integration/analyze_soak_results.sh` - Result analysis script
- `docs/soak_test_results.md` - Baseline results

**Dependencies**: Extension framework from Phase 7

**Estimated Effort**: 4-5 hours

---

#### T068: Performance Profiling & Dashboard [P]
**Goal**: Add monitoring and profiling capabilities for performance optimization

**Deliverables**:
- [ ] Memory profiling hooks (valgrind integration)
- [ ] CPU profiling hooks (perf integration)
- [ ] Performance dashboard endpoint (/metrics)
- [ ] Real-time metrics (memory, CPU, thread count)
- [ ] Historical metrics storage
- [ ] Alert thresholds for anomalies

**Files to Create/Modify**:
- `core/services/diagnostics/DiagnosticsEndpoint.cpp` - Add /metrics endpoint
- `core/services/profiling/ProfilerService.h` (new)
- `core/services/profiling/ProfilerService.cpp` (new)
- `ui/qml/screens/DiagnosticsScreen.qml` (new) - Metrics dashboard
- `scripts/run-with-profiling.sh` - Profiling wrapper

**Dependencies**: Diagnostics endpoint from Phase 2

**Estimated Effort**: 5-7 hours

---

### Documentation & Release

#### T069: Quickstart Validation
**Goal**: Verify quickstart guide works on clean WSL installation

**Deliverables**:
- [ ] Test setup on fresh WSL Ubuntu 22.04
- [ ] Verify all build steps
- [ ] Test all benchmark scripts
- [ ] Document any issues found
- [ ] Update quickstart with fixes

**Files to Update**:
- `specs/002-infotainment-androidauto/quickstart.md` - Verified and updated
- `docs/BUILD_VERIFICATION_REPORT.md` (new) - Detailed validation report

**Dependencies**: All previous phases

**Estimated Effort**: 3-4 hours

---

#### T070: Final README Documentation
**Goal**: Complete project documentation for public release

**Deliverables**:
- [ ] Feature summary
- [ ] Installation instructions (WSL, Raspberry Pi)
- [ ] Getting started guide
- [ ] Architecture overview
- [ ] Contributing guidelines
- [ ] License information
- [ ] Links to detailed documentation

**Files to Create/Modify**:
- `README.md` (update) - Complete project README
- `docs/INSTALLATION.md` (new) - Detailed installation guide
- `docs/CONTRIBUTING.md` (new) - Contributing guidelines
- `CHANGELOG.md` (new) - Version history

**Dependencies**: All phases

**Estimated Effort**: 3-4 hours

---

## Implementation Priority & Dependencies

### Execution Order (Recommended)

1. **Week 1 - Security & Foundation**
   - T061: Secure WebSocket (wss)
   - T062: Structured JSON logging
   - T063: Contract versioning (documentation)

2. **Week 1-2 - Compliance & UX**
   - T064: Design for Driving compliance
   - T065: Localization expansion
   - T066: Driving mode safety

3. **Week 2 - Testing & Validation**
   - T067: 24-hour soak test
   - T068: Performance profiling
   - T069: Quickstart validation

4. **Week 2 - Release**
   - T070: Final README

### Parallel Opportunities

All 10 tasks marked **[P]** can run in parallel (different files, no dependencies):
- T061, T062, T063 - Can all start simultaneously
- T064, T065, T066 - Can all start simultaneously
- T067, T068, T069, T070 - Can all start simultaneously

**Recommended**: Single developer can complete in 3-5 days sequentially, or 2 developers in 2-3 days with parallelization.

---

## Testing Strategy for Phase 8

### Regression Testing
- ✅ Ensure all Phase 1-7 tests still pass
- ✅ Run full test suite after each task
- ✅ No existing functionality should break

### New Tests for Phase 8
- T061: Test wss connection, certificate validation
- T062: Verify JSON log format, all services logging
- T063: Documentation review (manual)
- T064: Measure tap target sizes, check contrast ratios
- T065: Test locale switching, verify all strings translated
- T066: Test driving mode activation/restrictions
- T067: Run soak test, monitor for 24 hours, analyze results
- T068: Test metrics endpoint, verify memory tracking
- T069: Manual validation on clean system
- T070: Documentation review (manual)

### Success Criteria
- ✅ All Phase 1-7 tests continue to pass
- ✅ New tests validate Phase 8 features
- ✅ Soak test completes without crashes (T067)
- ✅ Documentation is complete and accurate
- ✅ Overall test pass rate ≥95% (60+/65 tests)

---

## Acceptance Criteria

### General
- [ ] All 10 tasks (T061-T070) marked complete in tasks.md
- [ ] All new code follows Google C++ / Qt best practices
- [ ] All files have copyright headers
- [ ] All new services integrated into CMakeLists.txt
- [ ] Build succeeds with zero errors
- [ ] All tests pass (≥95% pass rate)

### Specific to Phase 8
- [ ] wss:// connection works alongside ws://
- [ ] All services log in JSON format
- [ ] Contract versioning policy documented
- [ ] UI meets Design for Driving standards
- [ ] 4+ languages supported with dynamic switching
- [ ] Driving mode restricts unsafe features
- [ ] Soak test runs for 24 hours without crashes
- [ ] Performance metrics dashboard operational
- [ ] Quickstart works on clean WSL
- [ ] README comprehensive and up-to-date

---

## Risk & Mitigation

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|-----------|
| wss certificate generation fails | Low | Medium | Test on clean system, document fallback |
| Logging breaks existing code | Medium | High | Add logging gradually, test after each service |
| Soak test exposes memory leak | Medium | High | Use valgrind, fix leaks before Phase 8 ends |
| Driving mode blocks safe features | Medium | High | Careful review, configurable safety levels |
| Localization missing strings | Medium | Low | Use linting tool to find untranslated strings |

---

## Success Metrics

### Quality Gates (All must pass)
✅ Build succeeds with zero errors  
✅ ≥95% test pass rate (60+/65 tests)  
✅ All performance benchmarks maintained from Phase 7  
✅ No memory leaks detected (soak test)  
✅ All UI meets accessibility standards  
✅ Complete documentation  

### Optional Improvements
- Code coverage reporting (>80% coverage)
- Automated CI/CD pipeline
- Performance profiling graphs
- Localization coverage reports

---

## Timeline Estimate

| Task | Time | Cumulative |
|------|------|-----------|
| T061 (wss) | 5h | 5h |
| T062 (Logging) | 7h | 12h |
| T063 (Versioning) | 2.5h | 14.5h |
| T064 (Driving UI) | 4.5h | 19h |
| T065 (Localization) | 3.5h | 22.5h |
| T066 (Driving Mode) | 5.5h | 28h |
| T067 (Soak Test) | 4.5h | 32.5h |
| T068 (Profiling) | 6h | 38.5h |
| T069 (Quickstart) | 3h | 41.5h |
| T070 (README) | 3.5h | 45h |
| **Total** | **45 hours** | **~5-6 working days** |

---

## Next Actions

1. ✅ Update tasks.md to reflect Phase 8 start
2. ✅ Create Phase 8 planning document (this file)
3. Create todo list for Phase 8 task tracking
4. Begin T061: Secure WebSocket (wss) implementation
5. Run full test suite after each task
6. Commit work regularly with clear messages

---

## Resources & References

### Documentation
- `docs/project_completion_summary.md` - Overall project context
- `docs/PHASES_4_5_6_IMPLEMENTATION_SUMMARY.md` - Earlier phases
- `QUICK_REFERENCE.md` - Build and test commands

### Design Standards
- Google Design for Driving: https://developers.google.com/cars (reference in docs)
- WCAG 2.1 Accessibility: https://www.w3.org/WAI/WCAG21/quickref/
- Qt Best Practices: https://doc.qt.io/qt-6/qml-best-practices.html

### Tools
- clang-tidy: Code quality
- valgrind: Memory profiling
- perf: CPU profiling
- lrelease: Translation compilation
- clang-format: Code formatting

---

**Status**: ✅ **Phase 8 Planning Complete - Ready to Begin Implementation**

Next: T061 Secure WebSocket (wss) Support
