# Fix Summary: Make tests optional to unblock CI packaging

- Date: 2025-12-18
- Affected area: CI Docker packaging for `crankshaft.core` (multi-arch) and top-level CMake.

## Root Cause
The Docker packaging build failed while compiling the `test_websocket` target because `AndroidAutoService.cpp` includes `MockAndroidAutoService.h`, which requires `QImage` from Qt6Gui. The Docker builder image was building tests even though `-DBUILD_TESTS=OFF` was passed, because the top-level `CMakeLists.txt` unconditionally enabled testing and added the `tests` subdirectory.

Error observed in logs:
- `fatal error: QImage: No such file or directory`
- Followed by `CPack Error: Problem running install command: ... --target preinstall` due to the failing build step.

## Change
- Introduced `BUILD_TESTS` CMake option (default: `ON`).
- Guarded `enable_testing()` and `add_subdirectory(tests)` with `if(BUILD_TESTS)`.

This respects the Docker builder’s `-DBUILD_TESTS=OFF`, preventing test targets from building during packaging.

## Outcome
- CI Docker packaging no longer attempts to compile tests, avoiding the QtGui dependency in headless builds.
- Local development still builds and runs tests by default (`BUILD_TESTS=ON`).

## Follow-ups
- If we later want to run tests inside CI Docker, either:
  - Add Qt6Gui dev packages explicitly and ensure appropriate X/eglfs headless configs, or
  - Provide lightweight test shims for GUI-heavy components to keep test builds headless.

## Files touched
- `CMakeLists.txt` (top level): add `option(BUILD_TESTS ...)` and wrap testing logic.

