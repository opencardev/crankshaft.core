# Crankshaft MVP - Automotive Infotainment System

A minimal viable product demonstrating a modern automotive infotainment system with separate Core (backend) and UI (frontend) processes communicating via WebSockets and EventBus.

## Architecture

```
Core (C++/Qt6) ◄──── WebSocket ────► UI (QML/Qt6)
  - EventBus                           - WebSocketClient
  - WebSocketServer                    - Theme System
  - Services                           - i18n Support
```

## Quick Start

### Prerequisites

- Qt6 >= 6.5.3 (Core, Network, WebSockets, Gui, Qml, Quick)
- CMake >= 3.16
- C++17 compiler
- Ninja or Make

### Build (Linux/WSL)

```bash
# Configure
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Build
cmake --build build -j

# Run tests
ctest --test-dir build --output-on-failure
```

### Run

```bash
# Terminal 1: Start Core
./build/core/crankshaft-core --port 8080

# Terminal 2: Start UI
./build/ui/crankshaft-ui --server ws://localhost:8080
```

### Development

```bash
# Format code
bash scripts/format.sh

# Check formatting
bash scripts/lint.sh

# Run tests
bash scripts/run_tests.sh

# Build package
cd build && cpack -G DEB
```

## Features

- ✅ EventBus with thread-safe pub/sub
- ✅ WebSocket communication (Core ↔ UI)
- ✅ JSON message protocol
- ✅ Theme system (light/dark mode)
- ✅ Internationalisation (en-GB, de-DE)
- ✅ Material Design Icons
- ✅ Unit and integration tests
- ✅ CI/CD pipeline (multi-arch DEB builds, SBOM generation)
- ✅ About page with acknowledgements, SBOM and changelog links

## Recent Enhancements

- Multi-architecture CI/CD workflows (amd64, arm64, armhf) producing DEB packages
- Automated CycloneDX SBOM generation published with builds/releases
- About page enhancements with acknowledgements and external links
- New Settings UI components (link/header) for richer About content
- Changelog generation workflow and documented changelog in `docs/CHANGELOG.md`

## Documentation

- [Architecture](docs/ARCHITECTURE.md)
- [WebSocket API](docs/API.md)
- [Development Guide](docs/DEVELOPMENT.md)
- [Changelog](docs/CHANGELOG.md)

## License

GPL v3 - See LICENSE file for details.
