# 24-Hour Soak Test Guide

**Purpose**: Validate system stability, detect memory leaks, and verify long-term performance under continuous operation  
**Script**: `tests/integration/soak_test.sh`  
**Duration**: 24 hours (configurable)  
**Coverage**: Core service + 3 sample extensions

---

## Overview

The 24-hour soak test is an integration test that runs the Crankshaft core service along with sample extensions continuously for an extended period. It monitors resource usage (memory, CPU), logs errors, and generates a comprehensive stability report.

### Test Objectives

1. **Stability Validation**: Verify all processes remain running for 24 hours
2. **Memory Leak Detection**: Monitor memory growth patterns
3. **CPU Usage Analysis**: Identify CPU spikes or sustained high utilization
4. **Error Logging**: Count and categorize errors during extended operation
5. **Performance Baseline**: Establish resource usage norms for production

---

## Quick Start

### Run Standard 24-Hour Test

```bash
# From project root
./tests/integration/soak_test.sh
```

### Run Shorter Test (for development)

```bash
# 1-hour test
./tests/integration/soak_test.sh 1

# 30-minute test (0.5 hours)
./tests/integration/soak_test.sh 0.5
```

### Monitor Progress

```bash
# Watch live log output
tail -f soak-logs/soak_test.log

# Check core service logs
tail -f soak-logs/core.log
```

---

## Test Architecture

### Components Under Test

```
┌─────────────────────────────────────┐
│   Soak Test Orchestrator            │
│   (tests/integration/soak_test.sh)  │
└──────────┬──────────────────────────┘
           │
           ├──> Core Service (crankshaft-core)
           │    - WebSocket server
           │    - Extension manager
           │    - Diagnostics API
           │
           ├──> Extension 1 (Mock Media Player)
           │    - Simulates media extension
           │
           ├──> Extension 2 (Mock Navigation)
           │    - Simulates navigation extension
           │
           └──> Extension 3 (Mock Phone)
                - Simulates phone extension
```

### Monitoring Strategy

- **Sample Interval**: 60 seconds (configurable)
- **Metrics Collected**:
  - Process memory (RSS in MB)
  - Process CPU utilization (%)
  - Error count from logs
  - Process alive/dead state

### Thresholds

| Metric | Threshold | Action |
|--------|-----------|--------|
| Memory | 2048 MB | Warning logged |
| CPU | 80% | Warning logged |
| Process Death | Any | Test fails immediately |
| Errors | >0 | Warning in report |

---

## Test Execution

### Prerequisites

1. **Build Complete**: Run `./scripts/build.sh` first
2. **Core Executable**: `build/core/crankshaft-core` must exist
3. **Disk Space**: Ensure ~500 MB free for logs
4. **Time Available**: 24 hours (or configured duration)

### Execution Flow

```
1. Setup
   ├─> Create log directory (soak-logs/)
   ├─> Verify build artifacts
   └─> Clear old logs

2. Start Services
   ├─> Launch core service
   ├─> Wait 3 seconds for initialization
   ├─> Launch 3 sample extensions
   └─> Verify all processes started

3. Monitor Loop (24 hours)
   ├─> Check process health every 60s
   ├─> Collect memory/CPU metrics
   ├─> Log warnings if thresholds exceeded
   ├─> Count errors in core log
   └─> Update progress every hour

4. Generate Report
   ├─> Calculate statistics (avg, min, max)
   ├─> Analyze memory leak patterns
   ├─> Determine pass/fail/warning status
   └─> Write markdown report

5. Cleanup
   ├─> Gracefully stop core service (SIGTERM)
   ├─> Stop all extensions
   └─> Final status logged
```

### Exit Codes

| Code | Meaning |
|------|---------|
| 0 | Test passed (all processes stable) |
| 1 | Test failed (process died or setup error) |

---

## Metrics and Analysis

### Memory Profiling

The test tracks memory usage (RSS) for each process:

```
Core Memory Profile:
  Start:  120 MB
  Middle: 125 MB (4% growth)
  End:    128 MB (6% growth)
  
  Status: ✅ STABLE (growth <20%)
```

#### Memory Leak Detection

A memory leak is suspected if:
- Memory growth >20% over test duration
- Consistent upward trend without plateau
- Memory continues growing in second half of test

**Investigation Tools** (if leak detected):
- Valgrind: `valgrind --leak-check=full --show-leak-kinds=all ./build/core/crankshaft-core`
- Heaptrack: `heaptrack ./build/core/crankshaft-core`
- ASan (Address Sanitizer): Rebuild with `-fsanitize=address`

### CPU Profiling

CPU usage is sampled every 60 seconds:

```
Core CPU Profile:
  Average: 15.3%
  Peak:    45.2% (during AA connection)
  Baseline: 8-12% (idle)
  
  Status: ✅ EFFICIENT (avg <80%)
```

#### CPU Spike Analysis

High CPU usage (>80%) may indicate:
- Busy-wait loops
- Inefficient polling
- Excessive logging
- Unoptimized algorithms

**Investigation Tools**:
- perf: `perf record -g -p <pid>` then `perf report`
- gprof: Rebuild with `-pg` flag
- Qt Creator Profiler: Attach to running process

### Error Analysis

Errors are counted from log files:

```bash
# Count errors
grep -c "ERROR" soak-logs/core.log

# View error details
grep "ERROR" soak-logs/core.log | head -20
```

Common error categories:
- **Connection errors**: Client disconnect, timeout
- **Resource errors**: File not found, permission denied
- **Logic errors**: Invalid state, assertion failure
- **External errors**: Android Auto protocol errors

---

## Interpreting Results

### Test Result Classifications

#### ✅ **PASS** (All Green)
```
- Zero process crashes
- Zero errors logged
- Memory growth <10%
- CPU average <50%
- All thresholds met
```
**Action**: Ready for production deployment

#### ⚠️ **PASS WITH WARNINGS** (Yellow)
```
- All processes stable
- <10 errors logged (non-critical)
- Memory growth 10-20%
- CPU spikes <90%
```
**Action**: Review warnings, consider extended test (48 hours)

#### ❌ **FAIL** (Red)
```
- Any process crashed
- >50 errors logged
- Memory growth >20%
- Sustained CPU >90%
```
**Action**: Debug issues before proceeding

### Report Sections

The generated report (`docs/SOAK_TEST_RESULTS.md`) includes:

1. **Executive Summary**: Pass/fail status, key findings
2. **Stability Metrics**: Process uptime, error count
3. **Resource Usage Statistics**: Memory/CPU tables
4. **Memory Profile**: Growth analysis, leak detection
5. **Performance Analysis**: Response times, efficiency
6. **Logs and Diagnostics**: File locations, error grep commands
7. **Recommendations**: Next steps based on results
8. **Test Environment**: Build info, platform details

---

## Common Issues and Troubleshooting

### Issue: Core service fails to start

**Symptoms**:
```
ERROR: Core service failed to start
ERROR: Core service process died unexpectedly (PID 12345)
```

**Solutions**:
1. Check build exists: `ls -lh build/core/crankshaft-core`
2. Run manually to see error: `./build/core/crankshaft-core --log-level=DEBUG`
3. Check dependencies: `ldd build/core/crankshaft-core`
4. Verify Qt installation: `qmake --version`

### Issue: Memory leak detected

**Symptoms**:
```
⚠️ Potential Memory Leak Detected
- Memory growth rate: 35% over test duration
```

**Solutions**:
1. Run with Valgrind:
   ```bash
   valgrind --leak-check=full --show-leak-kinds=all \
     --log-file=valgrind.log \
     ./build/core/crankshaft-core
   ```

2. Run with Heaptrack:
   ```bash
   heaptrack ./build/core/crankshaft-core
   heaptrack_gui heaptrack.crankshaft-core.<pid>.gz
   ```

3. Rebuild with ASan:
   ```bash
   ./scripts/build.sh --build-type Debug --sanitize address
   ./tests/integration/soak_test.sh 1  # 1-hour test
   ```

4. Check common leak sources:
   - Qt signals/slots not disconnected
   - WebSocket connections not closed
   - Extension processes not terminated
   - Circular references in shared_ptr

### Issue: CPU usage too high

**Symptoms**:
```
WARNING: Core CPU exceeds threshold: 85.3% > 80%
```

**Solutions**:
1. Profile with perf:
   ```bash
   # Record for 30 seconds
   sudo perf record -g -p $(pgrep crankshaft-core) -- sleep 30
   sudo perf report
   ```

2. Check for busy-wait loops:
   ```bash
   grep -r "while.*{" core/ | grep -v "// "
   ```

3. Verify event loop efficiency:
   - Qt event loop not blocked
   - No synchronous network I/O in main thread
   - WebSocket processing asynchronous

4. Review logging verbosity:
   - Reduce log level in production
   - Use structured logging efficiently

### Issue: Extensions crash

**Symptoms**:
```
ERROR: Extension 1 process died unexpectedly (PID 12346)
```

**Solutions**:
1. Replace mock extensions with real extension executables
2. Check extension manifest permissions
3. Verify cgroup resource limits not too restrictive
4. Review extension logs for crash details

---

## Advanced Configuration

### Custom Durations

```bash
# 48-hour extended test
./tests/integration/soak_test.sh 48

# 15-minute smoke test
./tests/integration/soak_test.sh 0.25
```

### Custom Thresholds

Edit script variables:

```bash
# In tests/integration/soak_test.sh
MAX_MEMORY_MB=4096    # Increase for larger systems
MAX_CPU_PERCENT=90    # Increase for burst tolerance
SAMPLE_INTERVAL=30    # Increase sample frequency
```

### Real Extensions

Replace mock extensions with actual executables:

```bash
# In start_sample_extensions() function
"${PROJECT_ROOT}/extensions/media/media-extension" &
EXT1_PID=$!

"${PROJECT_ROOT}/extensions/navigation/nav-extension" &
EXT2_PID=$!

"${PROJECT_ROOT}/extensions/phone/phone-extension" &
EXT3_PID=$!
```

---

## Integration with CI/CD

### GitHub Actions Example

```yaml
name: Soak Test

on:
  schedule:
    # Run nightly at 2 AM UTC
    - cron: '0 2 * * *'
  workflow_dispatch:
    inputs:
      duration:
        description: 'Test duration (hours)'
        required: false
        default: '24'

jobs:
  soak-test:
    runs-on: ubuntu-22.04
    timeout-minutes: 1500  # 25 hours (24h test + 1h buffer)
    
    steps:
      - uses: actions/checkout@v3
      
      - name: Install dependencies
        run: sudo apt-get update && sudo apt-get install -y qt6-base-dev cmake
      
      - name: Build project
        run: ./scripts/build.sh --build-type Release
      
      - name: Run soak test
        run: ./tests/integration/soak_test.sh ${{ github.event.inputs.duration || '24' }}
      
      - name: Upload results
        uses: actions/upload-artifact@v3
        if: always()
        with:
          name: soak-test-results
          path: |
            docs/SOAK_TEST_RESULTS.md
            soak-logs/
      
      - name: Check test status
        run: |
          if grep -q "✅ PASS" docs/SOAK_TEST_RESULTS.md; then
            echo "Soak test passed"
            exit 0
          else
            echo "Soak test failed or has warnings"
            exit 1
          fi
```

### Jenkins Pipeline Example

```groovy
pipeline {
    agent any
    
    triggers {
        cron('0 2 * * *')  // Nightly at 2 AM
    }
    
    parameters {
        string(name: 'DURATION_HOURS', defaultValue: '24', description: 'Test duration')
    }
    
    stages {
        stage('Build') {
            steps {
                sh './scripts/build.sh --build-type Release'
            }
        }
        
        stage('Soak Test') {
            steps {
                sh "./tests/integration/soak_test.sh ${params.DURATION_HOURS}"
            }
        }
        
        stage('Archive Results') {
            steps {
                archiveArtifacts artifacts: 'docs/SOAK_TEST_RESULTS.md, soak-logs/**', 
                                 fingerprint: true
            }
        }
    }
    
    post {
        always {
            publishHTML([
                reportDir: 'docs',
                reportFiles: 'SOAK_TEST_RESULTS.md',
                reportName: 'Soak Test Report'
            ])
        }
    }
}
```

---

## Performance Baselines

### Expected Resource Usage (Raspberry Pi 4, 4GB RAM)

| Component | Memory (Idle) | Memory (Active) | CPU (Idle) | CPU (Active) |
|-----------|---------------|-----------------|------------|--------------|
| Core Service | 80-120 MB | 150-200 MB | 5-10% | 15-30% |
| Media Extension | 30-50 MB | 60-100 MB | 2-5% | 10-20% |
| Nav Extension | 40-60 MB | 80-120 MB | 3-8% | 15-25% |
| Phone Extension | 20-40 MB | 40-60 MB | 1-3% | 5-10% |
| **Total** | **~200 MB** | **~400 MB** | **~15%** | **~50%** |

### Expected Resource Usage (AMD64, 8GB RAM)

| Component | Memory (Idle) | Memory (Active) | CPU (Idle) | CPU (Active) |
|-----------|---------------|-----------------|------------|--------------|
| Core Service | 100-150 MB | 200-300 MB | 2-5% | 8-15% |
| Extensions (3x) | 80-120 MB | 150-250 MB | 3-8% | 10-20% |
| **Total** | **~250 MB** | **~500 MB** | **~10%** | **~30%** |

---

## Next Steps

After successful soak test completion:

1. ✅ **T067 Complete**: 24-hour stability validated
2. ⏳ **T068**: Implement performance profiling dashboard
3. ⏳ **T069**: Validate quickstart on clean system
4. ⏳ **T070**: Write final README documentation

---

## References

- **Script**: `tests/integration/soak_test.sh`
- **Report Template**: `docs/SOAK_TEST_RESULTS.md` (generated)
- **Core Service**: `build/core/crankshaft-core`
- **Extension Framework**: `docs/EXTENSION_FRAMEWORK.md`
- **Logging Guide**: `docs/STRUCTURED_JSON_LOGGING.md`

---

*Guide version: 1.0*  
*Last updated: January 3, 2026*  
*Task: T067 - 24-Hour Soak Test*
