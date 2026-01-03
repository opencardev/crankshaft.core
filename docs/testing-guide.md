# CTest Configuration and CI Integration

## Overview
This document describes the CTest configuration for Crankshaft MVP, which provides automated testing infrastructure for unit tests, contract tests, and integration tests.

## Test Organization

### Unit Tests
Located in `tests/unit/`, these test individual components in isolation:

- **test_websocket_validation.cpp**: Tests WebSocket message validation logic
  - Validates subscribe/unsubscribe/publish/service_command message types
  - Validates required fields per message type
  - Tests command allowlist enforcement (5 allowed service commands)
  - 28 test cases covering valid and invalid inputs

- **test_contract_schemas.cpp**: Tests JSON schema contracts
  - Validates WebSocket message contract structure
  - Validates extension manifest schema requirements
  - SimpleJsonSchemaValidator utility for schema validation without external dependencies

### Integration Tests
Located in `tests/`, these test component interactions:

- **test_websocket.cpp**: WebSocketServer integration tests
  - Tests message send/receive functionality
  - Tests EventBus integration
  - Tests service manager interaction

- **test_eventbus.cpp**: EventBus functionality tests
  - Tests publish/subscribe patterns
  - Tests client filtering

## Running Tests

### Command Line
```bash
# Run all tests
ctest --test-dir build --output-on-failure

# Run specific test
ctest --test-dir build -R WebSocketValidationTest --output-on-failure

# Run with verbose output
ctest --test-dir build --verbose

# Run tests in parallel
ctest --test-dir build --parallel 4
```

### VS Code Tasks
```bash
# Ctrl+Shift+B, select "Run Tests"
# Depends on: Build (Debug)
```

### Build System Integration
```bash
# Build and run tests in sequence
./scripts/build.sh --build-type Debug && ctest --test-dir build --output-on-failure
```

## Test Execution Flow

1. **CMake Configuration Phase** (`cmake -B build`)
   - Discovers all test executables in tests/CMakeLists.txt
   - Generates CTestTestfile.cmake in build/ directory
   - Registers test targets with ctest

2. **Build Phase** (`cmake --build build`)
   - Compiles test_websocket_validation
   - Compiles test_contract_schemas
   - Compiles test_websocket (integration test)
   - Compiles test_eventbus (integration test)
   - Outputs binaries to build/tests/

3. **Test Execution Phase** (`ctest --test-dir build`)
   - Discovers tests from CTestTestfile.cmake
   - Runs each test executable
   - Captures stdout/stderr
   - Returns aggregated pass/fail status

## Test Naming Convention

Test names follow the pattern: `<Component>Test` or `<Feature>Test`

- WebSocketValidationTest (unit)
- ContractSchemasTest (unit)
- WebSocketTest (integration)
- EventBusTest (integration)

## Assertions and Matchers

Tests use Qt Test Framework assertions:

```cpp
QVERIFY(condition)              // Assert true
QVERIFY2(condition, message)    // Assert with error message
QCOMPARE(actual, expected)      // Assert equality
QTEST_MAIN(TestClass)          // Entry point macro
```

## Exit Codes

- **0**: All tests passed
- **1**: At least one test failed
- **Other**: Test infrastructure error

## CI/CD Pipeline Integration

### GitHub Actions Workflow
```yaml
- name: Run Tests
  run: |
    ctest --test-dir build --output-on-failure
```

### Jenkins Pipeline
```groovy
stage('Test') {
  steps {
    sh 'ctest --test-dir build --output-on-failure'
  }
}
```

## Performance Notes

- Unit tests (WebSocket validation, contract schemas): < 100ms total
- Integration tests (WebSocket, EventBus): 200-500ms total
- All tests should complete in < 1s on modern hardware

## Adding New Tests

1. Create test file in `tests/unit/` or `tests/`
2. Use Qt Test Framework: `#include <QTest>`
3. Subclass `QObject` with `Q_OBJECT`
4. Add test slots with `private slots:`
5. Register in CMakeLists.txt:
   ```cmake
   add_executable(test_myfeature
     test_myfeature.cpp
   )
   target_link_libraries(test_myfeature PRIVATE Qt6::Core Qt6::Test)
   add_test(NAME MyFeatureTest COMMAND test_myfeature)
   ```

## Debugging Test Failures

### Run Single Test with Debug Output
```bash
ctest --test-dir build -R TestName -VV
```

### Run Test Directly in GDB
```bash
gdb ./build/tests/test_myfeature
(gdb) run
```

### Enable Qt Debug Logging
```bash
QT_LOGGING_RULES="*=true" ctest --test-dir build -R TestName
```

## Code Coverage

Currently, tests are configured for basic validation. Code coverage reporting can be enabled with CMake flags:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON
cmake --build build
ctest --test-dir build --output-on-failure
# Coverage reports in build/coverage/
```

## Test Strategy Alignment

Tests implement the Test-First principle from the project constitution:

1. **Unit Tests** (T011): Validate individual components
   - WebSocket message validation logic
   - Schema validation utilities

2. **Contract Tests** (T012-T013): Validate component contracts
   - WebSocket message schema compliance
   - Extension manifest structure requirements

3. **Integration Tests** (Pre-existing): Validate component interactions
   - EventBus publish/subscribe
   - WebSocketServer with EventBus
   - Service manager lifecycle

4. **User Story Tests** (Phase 3+): Validate end-to-end scenarios
   - Android Auto connection flow
   - Media player control
   - Settings management
   - Extension lifecycle

## Maintenance

### Regular Updates
- Ensure new features have corresponding tests
- Update tests when API contracts change
- Review test coverage monthly
- Archive test results for trend analysis

### Troubleshooting
- If CMake can't find Qt6::Test, run: `cmake --fresh -B build`
- If tests don't run, check that `enable_testing()` is in tests/CMakeLists.txt
- If test output is truncated, redirect to file: `ctest --test-dir build --output-on-failure > test_results.txt`
