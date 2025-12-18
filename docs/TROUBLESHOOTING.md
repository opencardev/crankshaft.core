# Troubleshooting Guide

A concise checklist to diagnose common Crankshaft issues on Raspberry Pi OS/WSL. All commands assume a shell on the target device (use `wsl bash -lc "<cmd>"` from Windows if needed).

## Quick checks
- Confirm services are active: `systemctl status crankshaft-core crankshaft-ui`.
- Inspect recent logs: `journalctl -u crankshaft-core -u crankshaft-ui -n 200 -r`.
- Verify WebSocket link: `ss -lntp | grep 8080` (default Core port).
- Check display backend: `echo $QT_QPA_PLATFORM` and ensure it matches hardware (`eglfs` on device, `vnc` for remote debugging).
- Free space and permissions: `df -h /` and `ls -ld /var/lib/crankshaft /var/log/crankshaft`.

## Services and process health
- Start/enable:
  ```bash
  sudo systemctl daemon-reload
  sudo systemctl enable --now crankshaft-core crankshaft-ui
  sudo systemctl status crankshaft-core crankshaft-ui
  ```
- Restart after config changes:
  ```bash
  sudo systemctl restart crankshaft-core crankshaft-ui
  ```
- If UI fails to start, ensure Core is running and reachable on its WebSocket port; UI unit requires Core.
- For repeated crashes, review `journalctl -u crankshaft-core -u crankshaft-ui -b` and look for missing plugins or permission errors.
- Check which services are running based on ProfileManager configuration:
  ```bash
  journalctl -u crankshaft-core -b | grep "Starting.*service"
  journalctl -u crankshaft-core -b | grep "Services running"
  ```
- Core uses the active host profile to determine which services to start (AndroidAuto, WiFi, Bluetooth). Check profile device configs if a service isn't starting:
  ```bash
  # Profile devices are logged at startup
  journalctl -u crankshaft-core -b | grep "Processing device"
  ```

## Logs and debug output
- Systemd journals (most reliable):
  ```bash
  journalctl -u crankshaft-core -u crankshaft-ui -b -e
  ```
- Persistent files (if configured): `/var/log/crankshaft/` for rotated logs.
- Enable verbose Qt logging temporarily:
  ```bash
  QT_LOGGING_RULES="*=true" QT_DEBUG_PLUGINS=1 crankshaft-ui
  ```
  For UI running under systemd, drop the env into an override:
  ```bash
  sudo systemctl edit crankshaft-ui
  # Add under [Service]:
  # Environment="QT_LOGGING_RULES=*=true" "QT_DEBUG_PLUGINS=1"
  sudo systemctl restart crankshaft-ui
  ```
- Android Auto stack (AASDK) extra logging (if relevant):
  ```bash
  export AASDK_LOG_LEVEL=DEBUG
  ```

## Connectivity: Core ↔ UI
- Default Core listen address: `ws://localhost:8080`. Confirm:
  ```bash
  ss -lntp | grep 8080
  curl -v ws://localhost:8080  # or use wscat if installed
  ```
- UI override flag (example): `./crankshaft-ui --server ws://<host>:8080`.
- If using another port, align `config/crankshaft.json` and the UI launch arguments.

## Display and input
- EGL/DRM on device: ensure `/dev/dri/card0` is accessible to user `crankshaft` and that `QT_QPA_PLATFORM=eglfs` (set in `crankshaft-ui.service`).
- Remote/VNC debugging: override platform when testing:
  ```bash
  QT_QPA_PLATFORM=vnc:size=1024x600,port=5900 crankshaft-ui
  ```
- If touch/input is unresponsive, check `udevadm monitor --udev` for device events and permissions under `/dev/input/`.

## Missing dependencies
- Core/UI runtime essentials:
  ```bash
  sudo apt-get update
  sudo apt-get install -y \
    qt6-base-dev qt6-base-dev-tools qt6-declarative-dev qt6-websockets-dev \
    libasound2-dev gstreamer1.0-plugins-base gstreamer1.0-plugins-good \
    gstreamer1.0-plugins-bad gstreamer1.0-libav libdrm-dev
  ```
- If EGLFS errors mention GBM/KMS, ensure `mesa-utils` and the matching GPU drivers are present.
- For VNC platform usage, install `libvncserver-dev` (if building with VNC support).

## Build/packaging checks
- Reconfigure and rebuild after toolchain changes:
  ```bash
  cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
  cmake --build build -j
  ctest --test-dir build --output-on-failure
  ```
- Verify installed binaries match expected version:
  ```bash
  crankshaft-core --version
  crankshaft-ui --version
  dpkg -l | grep crankshaft
  ```

## Storage, permissions, and ownership
- Service user/group: `crankshaft`. Ensure writable dirs:
  ```bash
  sudo chown -R crankshaft:crankshaft /var/lib/crankshaft /var/log/crankshaft
  ```
- When running manually as another user, delete any stale lock files under `/var/lib/crankshaft`.

## Networking and time
- DNS/time issues can break HTTPS and WebSocket wss setups:
  ```bash
  timedatectl status
  ping -c1 github.com
  ```
- If using a proxy, export `http_proxy`/`https_proxy` for both Core and UI or set them in the systemd overrides.

## When to file a bug
- Capture: failing commands, full `journalctl -u crankshaft-core -u crankshaft-ui -b`, platform info (`uname -a`, `lsb_release -a`), and whether you used `QT_LOGGING_RULES=*=true QT_DEBUG_PLUGINS=1`.
- File the report with logs and steps to reproduce.
