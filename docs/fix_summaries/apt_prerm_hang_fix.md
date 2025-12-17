# Fix Summary: crankshaft-ui removal hang (dpkg/prerm)

## Overview
Some Raspberry Pi systems reported `dpkg` hanging at “Removing crankshaft-ui …”. The root cause was the package `prerm` script stopping the systemd unit synchronously. On misbehaving or absent units, `systemctl` can block until its timeout, effectively stalling the whole removal.

## Root Cause
- `prerm` used synchronous `systemctl stop` and `systemctl disable` during `remove`.
- On machines with a stuck unit (or odd systemd state), the stop/disable calls could block and never return.
- The script did not explicitly avoid blocking behaviour and did not time out operations.

## Changes Made
- Hardened `packaging/ui/prerm`:
  - Switched shebang to `/bin/sh` to follow Debian maintainer script conventions.
  - Added `DEBIAN_FRONTEND=noninteractive` and `set -eu`.
  - Guarded all systemd interactions: only run when systemd is present.
  - Used `systemctl stop --no-block` and `systemctl disable --no-reload` with short `timeout` wrappers.
  - Kept process cleanup via `pkill`, ensuring it never blocks and always best-effort.
  - Extended action case to cover `remove|upgrade|deconfigure`.

These changes ensure the script exits quickly and never blocks, even when the unit misbehaves or is missing.

## How to Unstick an Affected Device
If your device is currently stuck in a half-removed state (`rF`), you can bypass the old `prerm` and complete removal safely. Run on the Raspberry Pi:

```bash
# 1) Replace the prerm with a safe no-op
echo "#!/bin/sh\nexit 0" | sudo tee /var/lib/dpkg/info/crankshaft-ui.prerm >/dev/null
sudo chmod +x /var/lib/dpkg/info/crankshaft-ui.prerm

# 2) Retry removal (use reinstreq in case of half-installed state)
sudo dpkg --remove --force-remove-reinstreq crankshaft-ui

# 3) Optionally purge configuration files too
sudo dpkg --purge crankshaft-ui || true

# 4) Verify dpkg is healthy
sudo dpkg --audit || true
```

After removal succeeds, install the rebuilt package containing the hardened `prerm`.

## Rebuild and Install the Fixed Package
From this repository:

- Build and package (recommended):
  - VS Code task: `Build Package (DEB)`
  - Or via WSL:
    ```bash
    ./scripts/build.sh --build-type Release --package
    ```
- Transfer and install the resulting `.deb` on your device:
  ```bash
  sudo apt install -y ./crankshaft-ui_*_arm64.deb  # adjust arch/version
  ```

## Notes
- The service stop/disable during removal is best-effort and non-blocking; the unit will be stopped either by systemd asynchronously or by `pkill` as a fallback.
- We follow British English in documentation and keep maintainer scripts resilient and non-interactive.
