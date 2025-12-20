**Profile Manager**

This document describes the `ProfileManager` service, the structure of host and vehicle profiles, common device configuration keys and example JSON files you can edit or add to the runtime configuration directory.

**Files & Location**: `host_profiles.json`, `vehicle_profiles.json` stored in the directory passed to `ProfileManager` constructor (`ProfileManager(const QString& configDir, ...)`).

**How it works**:
- **ProfileManager** constructs with a `configDir` path. It will create the directory if missing and try to load `host_profiles.json` and `vehicle_profiles.json` from it.
- If no profiles are present, default development host and vehicle profiles are initialised automatically (see examples below).
- Profiles are persisted as JSON arrays; each array element is a profile object.

**Key Types**
- **DeviceConfig**: describes devices attached to a host profile.
  - `name` (string) — unique device name within the host profile (e.g. `AndroidAuto`, `Bluetooth`).
  - `type` (string) — device interface type, e.g. `AndroidAuto`, `Bluetooth`, `WiFi`.
  - `enabled` (bool) — whether the device is enabled for this host.
  - `useMock` (bool) — true = use the mock implementation; false = use real hardware.
  - `settings` (map/object) — key/value map for device-specific settings.
  - `description` (string) — freeform description.

- **HostProfile**: describes a machine/host configuration.
  - `id` (string) — UUID; if empty a new one will be generated when creating a profile.
  - `name` (string)
  - `description` (string)
  - `isActive` (bool)
  - `createdAt`, `modifiedAt` (ISO datetime strings when persisted)
  - `cpuModel`, `ramMB`, `osVersion` (informational)
  - `properties` (object) — host-level key/value map
  - `devices` (array of `DeviceConfig`)

- **VehicleProfile**: describes vehicle-specific configuration (used by VehicleHAL and UI).
  - `id`, `name`, `description`, `isActive`, `createdAt`, `modifiedAt`
  - `make`, `model`, `year`, `vin`, `licensePlate`, `vehicleType`
  - `supportedModes` (array of strings) — e.g. `PARK`, `REVERSE`, `DRIVE`
  - `hasAWD` (bool), `wheelCount` (integer)
  - `properties`, `mockDefaults` (objects)

**Common Device Settings (Android Auto example)**
- `connectionMode`: `auto` | `usb` | `wireless` — `auto` attempts USB first then wireless.
- `wireless.enabled` (bool) — enable Android Auto via TCP (developer option).
- `wireless.host` (string) — phone/host IP or hostname for wireless connection.
- `wireless.port` (int) — TCP port (default 5277).
- `channels.<name>` (bool) — toggle channels such as `channels.video`, `channels.mediaAudio`, `channels.microphone`.
- `generateTestVideo` / `generateTestAudio` (bool) — mock-specific test generators.

**ProfileManager API (useful methods)**
- Construct: `ProfileManager(const QString& configDir, QObject* parent=nullptr)` — pass a directory where JSON files live.
- `bool createHostProfile(const HostProfile& profile)` — create and persist.
- `bool updateHostProfile(const HostProfile& profile)` — update and persist.
- `bool deleteHostProfile(const QString& profileId)`
- `HostProfile getHostProfile(const QString& profileId) const` — returns a copy.
- `QList<HostProfile> getAllHostProfiles() const`
- `bool setActiveHostProfile(const QString& profileId)` — marks profile active and persists; emits `hostProfileChanged(profileId)`.
- `HostProfile getActiveHostProfile() const`
- Similarly for vehicle profiles: `createVehicleProfile`, `updateVehicleProfile`, `setActiveVehicleProfile`, `getActiveVehicleProfile`.
- Device helpers: `addDeviceToHostProfile`, `removeDeviceFromHostProfile`, `setDeviceEnabled`, `setDeviceUseMock`, `getProfileDevices`.

Signals:
- `hostProfileChanged(const QString& profileId)` — emitted when the active host profile changes or is updated.
- `vehicleProfileChanged(const QString& profileId)` — similar for vehicle.
- `deviceConfigChanged(const QString& profileId, const QString& deviceName)` — emitted on device changes.

**On-disk format**
- `host_profiles.json` is a JSON array of host profile objects.
- `vehicle_profiles.json` is a JSON array of vehicle profile objects.

Below are minimal example JSON files you can drop into the config directory used by `ProfileManager`.

**Example: `host_profiles.json`**

`[`
`  {`
`    "id": "00000000-0000-4000-8000-000000000001",
`    "name": "Development Host",
`    "description": "Development host with mock devices",
`    "isActive": true,
`    "cpuModel": "Intel i7 (Development)",
`    "ramMB": 16384,
`    "osVersion": "Raspberry Pi OS (Simulated)",
`    "devices": [
`      {
`        "name": "AndroidAuto",
`        "type": "AndroidAuto",
`        "enabled": true,
`        "useMock": true,
`        "description": "Android Auto projection service",
`        "settings": {
`          "resolution": "1024x600",
`          "fps": 30,
`          "generateTestVideo": true,
`          "channels.video": true,
`          "channels.mediaAudio": true,
`          "connectionMode": "auto",
`          "wireless.enabled": false,
`          "wireless.host": "",
`          "wireless.port": 5277
`        }
`      },
`      {
`        "name": "Bluetooth",
`        "type": "Bluetooth",
`        "enabled": true,
`        "useMock": true,
`        "description": "Bluetooth adapter"
`      }
`    ]
`  }
`]

**Example: `vehicle_profiles.json`**

`[`
`  {
`    "id": "00000000-0000-4000-8000-000000000010",
`    "name": "Test Vehicle - Sedan",
`    "description": "Generic test vehicle for development",
`    "isActive": true,
`    "make": "Test Make",
`    "model": "Test Model",
`    "year": "2025",
`    "vin": "TEST000000000001",
`    "licensePlate": "MOCK-001",
`    "vehicleType": "car",
`    "supportedModes": ["PARK","REVERSE","NEUTRAL","DRIVE"],
`    "hasAWD": true,
`    "wheelCount": 4
`  }
`]

**Editing tips & usage notes**
- Files are reloaded when you call `ProfileManager::loadProfiles()` — changes saved via `saveProfiles()`.
- For development you can set `useMock: true` for a device to avoid requiring physical hardware.
- To enable Android Auto wireless testing on a PC, set `wireless.enabled` to `true` and set `wireless.host`/`wireless.port` to the phone's address and port.
- Keep device `name` values unique within a host profile; `type` helps the ServiceManager map to the correct service implementation.

If you want, I can also:
- add a small CLI or script to generate new profiles from a template,
- add JSON schema validation and an example schema file (`host_profiles.schema.json`).

**JSON Schema files**
The repository includes JSON Schema files you can use to validate profile and configuration JSON files before using them at runtime. They are located in:

- `docs/schemas/host_profiles.schema.json` — schema for `host_profiles.json` (array of host profiles).
- `docs/schemas/vehicle_profiles.schema.json` — schema for `vehicle_profiles.json` (array of vehicle profiles).
- `docs/schemas/crankshaft_config.schema.json` — minimal schema for `crankshaft.json` configuration.

Example validation using `ajv-cli` (recommended) from WSL or a machine with Node.js:

```bash
# install once:
npm install -g ajv-cli

# validate host profiles (example path):
ajv validate -s docs/schemas/host_profiles.schema.json -d /path/to/config/host_profiles.json

# validate vehicle profiles:
ajv validate -s docs/schemas/vehicle_profiles.schema.json -d /path/to/config/vehicle_profiles.json

# validate crankshaft config:
ajv validate -s docs/schemas/crankshaft_config.schema.json -d /path/to/config/crankshaft.json
```

If you prefer Python, `jsonschema` can be used:

```bash
pip install jsonschema
python -m jsonschema -i /path/to/config/host_profiles.json docs/schemas/host_profiles.schema.json
```

**Profile CLI**
There is a small convenience CLI included in the repository to create example profiles and validate them using the JSON schemas:

- Script: `docs/tools/profile_cli.py`
- Requirements: `docs/tools/requirements.txt` (install with `pip install -r docs/tools/requirements.txt`).

Examples:

```bash
# create a host_profiles.json (single element array) for editing
python docs/tools/profile_cli.py create-host --name "Dev Host" --out /tmp/host_profiles.json

# validate the created file
python docs/tools/profile_cli.py validate --type host --file /tmp/host_profiles.json
```

The CLI supports `create-host`, `create-vehicle` and `validate --type host|vehicle|config`.


---
File created by documentation assistant.
