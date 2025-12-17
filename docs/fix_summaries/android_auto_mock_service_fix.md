# Android Auto Mock Service Always Used - Fix Summary

## Issue
User reported seeing device detection logs in `/var/log/crankshaft/usb_events.log` (Pixel_8_Pro detected) but no debug messaging from the Android Auto protocol stack, and the phone wasn't actually connecting.

## Root Cause
The factory function in `core/services/android_auto/AndroidAutoService.cpp` (lines 299–323) is hardcoded to always instantiate `MockAndroidAutoService` instead of `RealAndroidAutoService`. The mock service simulates connection state changes for UI testing but does not perform actual USB/AOAP protocol negotiation with the phone.

Relevant code snippet (before fix):
```cpp
AndroidAutoService* AndroidAutoService::create(MediaPipeline* mediaPipeline, QObject* parent) {
  QByteArray useMock = qgetenv("CRANKSHAFT_USE_MOCK_AA");
  bool mockEnabled = !useMock.isEmpty() && (useMock == "1" || useMock.toLower() == "true");

  AndroidAutoService* service = nullptr;

  if (mockEnabled) {
    Logger::instance().info("Creating Mock Android Auto service (CRANKSHAFT_USE_MOCK_AA=1)");
    service = new MockAndroidAutoService(parent);
  } else {
    Logger::instance().warning(
        "Real Android Auto service not yet available - using Mock. "
        "TODO: Fix AASDK header paths in RealAndroidAutoService");
    service = new MockAndroidAutoService(parent);  // <-- Always Mock
    // TODO: Uncomment when AASDK headers are fixed:
    // service = new RealAndroidAutoService(mediaPipeline, parent);
  }

  return service;
}
```

The AASDK headers are actually present and the build configuration is correct (`external/aasdk` submodule is configured and linked), so the TODO comment is outdated.

## Fix
1. Replace the hardcoded `MockAndroidAutoService` instantiation with logic that checks the ProfileManager's AndroidAuto device configuration.
2. Add ProfileManager parameter to the factory method signature.
3. Query the active host profile's AndroidAuto device config and use its `useMock` flag.
4. Rebuild and redeploy.

Changed code:
```cpp
// In AndroidAutoService.h
static AndroidAutoService* create(MediaPipeline* mediaPipeline, ProfileManager* profileManager,
                                   QObject* parent = nullptr);

// In AndroidAutoService.cpp
AndroidAutoService* AndroidAutoService::create(MediaPipeline* mediaPipeline,
                                                ProfileManager* profileManager, QObject* parent) {
  bool useMock = true;  // Default to mock if profile not found

  if (profileManager) {
    HostProfile activeProfile = profileManager->getActiveHostProfile();
    for (const auto& device : activeProfile.devices) {
      if (device.name == "AndroidAuto" || device.type == "AndroidAuto") {
        useMock = device.useMock;
        Logger::instance().info(
            QString("AndroidAuto device found in profile '%1': useMock=%2")
                .arg(activeProfile.name)
                .arg(useMock ? "true" : "false"));
        break;
      }
    }
  }

  AndroidAutoService* service = nullptr;

  if (useMock) {
    Logger::instance().info("Creating Mock Android Auto service (profile setting)");
    service = new MockAndroidAutoService(parent);
  } else {
    Logger::instance().info("Creating Real Android Auto service (AASDK)");
    service = new RealAndroidAutoService(mediaPipeline, parent);
  }

  return service;
}
```

**Configuration**: To use the real service, update the active host profile's AndroidAuto device config:
- Set `useMock: false` in the profile JSON, or
- Use ProfileManager API: `profileManager->setDeviceUseMock(profileId, "AndroidAuto", false)`

## Testing
After deploying the fix:
- By default, the ProfileManager's default host profile has `AndroidAuto` device with `useMock: true`.
- To enable the real AASDK service:
  ```bash
  # Option 1: Update profile via ProfileManager API (when exposed via WebSocket)
  # Set useMock to false for AndroidAuto device in the active profile
  
  # Option 2: Edit profile JSON directly (if profiles are persisted to disk)
  # Find the active host profile file and set:
  # "devices": [{"name": "AndroidAuto", "useMock": false, ...}]
  ```
- Set `AASDK_LOG_LEVEL=DEBUG` in the systemd service environment.
- Restart `crankshaft-core`.
- Plug in the phone and monitor `journalctl -u crankshaft-core -f`.
- Expect to see AASDK negotiation logs (AOAP switch, transport setup, channel creation, etc.) instead of just mock state changes.

## Files Changed
- `core/services/android_auto/AndroidAutoService.h` (added ProfileManager parameter to factory)
- `core/services/android_auto/AndroidAutoService.cpp` (replaced env var check with ProfileManager device config lookup)

## Related Documentation
- `docs/android_auto_troubleshooting.md` - updated with note about mock vs real service
- `docs/TROUBLESHOOTING.md` - general service and logging guidance

## Date
2025-12-17
