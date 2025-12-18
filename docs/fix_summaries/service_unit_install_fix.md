# Fix Summary: Service units not installed (systemd unit not found)

## Symptom
On target devices, `systemctl` reported unit not found for `crankshaft-ui` and/or `crankshaft-core` during removal or enablement:

- `systemctl stop crankshaft-ui` / `disable` failed with "Unit crankshaft-ui.service not found".
- Post-install did not enable the services as expected.

## Root Cause
The `.service` unit files were not part of the data payload of the DEB packages. They were referenced in `CPACK_DEBIAN_*_PACKAGE_CONTROL_EXTRA`, which only adds files into the control archive (used for maintainer scripts) and does not install them in the filesystem. As a result, the unit files never appeared in `/lib/systemd/system/`, and `systemctl` could not find them.

## Changes Made
- Added explicit CMake install rules to place units into `/lib/systemd/system`:
  - `core/CMakeLists.txt`: installs `packaging/core/crankshaft-core.service` (component `core`).
  - `ui/CMakeLists.txt`: installs `packaging/ui/crankshaft-ui.service` (component `ui`).
- Maintainer scripts remain responsible only for `daemon-reload` and enabling units, now guaranteed to exist.

## Outcome
- `systemctl enable crankshaft-*.service` succeeds during `postinst`.
- Units are present in `/lib/systemd/system/` allowing start/stop/disable to work.
- Removal scripts no longer fail due to missing units (and they are hardened to avoid blocking).

## Rebuild Steps
Use the workspace task or WSL to build and package updated DEBs:

```bash
./scripts/build.sh --build-type Release --package
```

Then (on the device):

```bash
sudo apt install -y ./crankshaft-core_*_arm64.deb ./crankshaft-ui_*_arm64.deb  # adjust versions/arch
```

This will install the units and enable them (systemd present).