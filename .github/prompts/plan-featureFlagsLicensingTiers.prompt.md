# Feature Flags with Licensing Tiers Implementation Plan

## Overview

A tiered feature flag system (Bronze, Silver, Gold, Platinum) that enables:
- Gradual rollout of new features without exposing them to everyone
- Licensing enforcement based on subscription/product tier
- Hardware-specific feature gating (RPi4 vs RPi5, mock vs real HAL)
- Silent feature introduction in main release builds
- Single codebase supporting all distribution channels

## Architecture

### 1. Feature Flag Schema & Storage

**Location**: `config/features.json` (new file)

```json
{
  "features": {
    "android-auto": {
      "name": "Android Auto Support",
      "description": "USB and wireless Android Auto projection",
      "minTier": "bronze",
      "rolloutPercentage": 100,
      "hardwareRequirements": {
        "requires": ["usb-otg"],
        "excludes": []
      },
      "enabled": true,
      "dependencies": []
    },
    "wireless-projection": {
      "name": "Wireless Android Auto",
      "description": "WiFi-based Android Auto without USB cable",
      "minTier": "silver",
      "rolloutPercentage": 50,
      "hardwareRequirements": {
        "requires": ["wifi6"],
        "excludes": []
      },
      "enabled": true,
      "dependencies": ["android-auto"]
    },
    "custom-radio-extensions": {
      "name": "Custom Radio Extensions",
      "description": "Community radio extensions and plugins",
      "minTier": "gold",
      "rolloutPercentage": 100,
      "hardwareRequirements": {
        "requires": [],
        "excludes": []
      },
      "enabled": true,
      "dependencies": []
    },
    "advanced-diagnostics": {
      "name": "Advanced Vehicle Diagnostics",
      "description": "OBD-II diagnostics, real-time telemetry, performance metrics",
      "minTier": "platinum",
      "rolloutPercentage": 100,
      "hardwareRequirements": {
        "requires": ["obd-adapter"],
        "excludes": []
      },
      "enabled": false,
      "dependencies": []
    },
    "multiple-display-support": {
      "name": "Multiple Display Support",
      "description": "Run UI on multiple displays simultaneously",
      "minTier": "platinum",
      "rolloutPercentage": 25,
      "hardwareRequirements": {
        "requires": [],
        "excludes": []
      },
      "enabled": true,
      "dependencies": []
    },
    "advanced-ui-themes": {
      "name": "Advanced UI Themes",
      "description": "Custom theme support and builder",
      "minTier": "silver",
      "rolloutPercentage": 100,
      "hardwareRequirements": {
        "requires": [],
        "excludes": []
      },
      "enabled": true,
      "dependencies": []
    },
    "bluetooth-mesh": {
      "name": "Bluetooth Mesh Networking",
      "description": "Connect and control mesh-enabled devices",
      "minTier": "gold",
      "rolloutPercentage": 0,
      "hardwareRequirements": {
        "requires": ["bluetooth-5"],
        "excludes": []
      },
      "enabled": true,
      "dependencies": []
    },
    "server-side-sync": {
      "name": "Server-Side Synchronisation",
      "description": "Sync settings, profiles, and data with cloud server",
      "minTier": "platinum",
      "rolloutPercentage": 10,
      "hardwareRequirements": {
        "requires": ["internet"],
        "excludes": []
      },
      "enabled": true,
      "dependencies": []
    }
  },
  "licensingTiers": {
    "bronze": {
      "name": "Bronze",
      "description": "Essential automotive features",
      "icon": "bronze"
    },
    "silver": {
      "name": "Silver",
      "description": "Enhanced connectivity and features",
      "icon": "silver"
    },
    "gold": {
      "name": "Gold",
      "description": "Advanced features and customisation",
      "icon": "gold"
    },
    "platinum": {
      "name": "Platinum",
      "description": "Premium tier with all features and beta features",
      "icon": "platinum"
    }
  }
}
```

### 2. License Tier Storage

**Location**: `config/crankshaft.json` (extend existing)

Add to root config:
```json
{
  "licensing": {
    "currentTier": "platinum",
    "licenseKey": "DEMO-xxxx-xxxx-xxxx",
    "hardwareId": "hash-of-mac-addresses",
    "validUntil": "2026-12-31T23:59:59Z",
    "allowOfflineGracePeriod": 30
  }
}
```

### 3. FeatureFlagService (New C++ Service)

**Location**: `core/services/feature_flags/`

**Header** (`FeatureFlagService.h`):
```cpp
#pragma once

#include <QString>
#include <QVariantMap>
#include <QJsonObject>
#include <QStringList>
#include <memory>

class FeatureFlagService {
    Q_OBJECT

public:
    enum class LicenseTier {
        Bronze = 0,
        Silver = 1,
        Gold = 2,
        Platinum = 3
    };

    static FeatureFlagService& instance();

    // Feature flag queries
    bool isFeatureEnabled(const QString& featureName) const;
    QVariantMap getFeature(const QString& featureName) const;
    QStringList getAllEnabledFeatures() const;
    QStringList getEnabledFeaturesForTier(LicenseTier tier) const;

    // License tier management
    LicenseTier getCurrentTier() const;
    void setCurrentTier(LicenseTier tier);
    QString getTierName(LicenseTier tier) const;

    // Hardware capability checking
    bool isHardwareCapabilityAvailable(const QString& capability) const;
    QStringList getAvailableHardwareCapabilities() const;

    // Rollout evaluation
    bool shouldEnableForRollout(const QString& featureName) const;
    int getRolloutPercentage(const QString& featureName) const;

    // Feature dependencies
    QStringList getFeatureDependencies(const QString& featureName) const;
    bool validateFeatureDependencies(const QString& featureName) const;

    // Debug/testing
    void forceFeatureEnabled(const QString& featureName, bool enabled);
    QJsonObject exportFeatureFlagsDebugInfo() const;

signals:
    void featureFlagsChanged();
    void tierChanged(LicenseTier oldTier, LicenseTier newTier);

private:
    FeatureFlagService();
    ~FeatureFlagService();

    void loadFeatureFlagsFromConfig();
    void loadLicenseFromConfig();
    QString generateDeviceHash() const;
    bool evaluateHardwareRequirements(const QJsonObject& feature) const;
    bool evaluateRollout(const QString& featureName) const;

    LicenseTier currentTier_;
    QString deviceId_;
    QJsonObject featureFlags_;
    QMap<QString, bool> forcedFlags_;  // For testing
};
```

**Implementation** (`FeatureFlagService.cpp`):
- Load feature flags from `config/features.json`
- Evaluate tier, hardware, rollout percentage, dependencies
- Provide thread-safe query API
- Generate deterministic device hash for rollout cohort assignment
- Integrate with existing ConfigService and ProfileManager

### 4. Integration Points

#### A. ServiceManager (`core/services/service_manager/ServiceManager.cpp`)

Before starting services, check if required features are enabled:
```cpp
void ServiceManager::startAllServices() {
    const auto& flags = FeatureFlagService::instance();

    for (const auto& service : servicesToStart) {
        // Check if service is feature-gated
        if (service == "AndroidAutoService" && !flags.isFeatureEnabled("android-auto")) {
            qDebug() << "Skipping AndroidAutoService (feature disabled)";
            continue;
        }
        // ... start service
    }
}
```

#### B. UI QML (`ui/qml/screens/SettingsScreen.qml`)

Show only enabled features in settings menus:
```qml
ColumnLayout {
    Repeater {
        model: wsClient.enabledFeatures  // Populated from WebSocket

        delegate: Rectangle {
            visible: isFeatureEnabled(model.name)
            // ... feature UI component
        }
    }
}
```

#### C. WebSocket API (`core/services/websocket/WebSocketServer.cpp`)

New endpoints:
```
GET /features                    → List all enabled features
GET /features/{name}             → Get feature details
POST /features/check             → Bulk check multiple features
GET /licensing/tier              → Get current license tier
SUBSCRIBE features/#             → Listen for feature changes
```

#### D. Extension Framework (Future)

In extension loader:
```cpp
bool ExtensionLoader::canLoadExtension(const QJsonObject& manifest) const {
    QString requiredTier = manifest.value("minTier").toString("bronze");
    LicenseTier required = parseTier(requiredTier);
    return FeatureFlagService::instance().getCurrentTier() >= required;
}
```

### 5. Hardware Detection & Capabilities

**Integrate with existing HAL** (`core/hal/HostHAL.{h,cpp}`):

Update `detectHardwareCapabilities()`:
```cpp
QStringList HostHAL::detectHardwareCapabilities() const {
    QStringList caps;

    if (isRaspberryPi()) {
        caps << "arm-linux-gnueabihf" << "usb-otg";
        if (getRPiVersion() >= 5) {
            caps << "wifi6" << "bluetooth-5";
        } else if (getRPiVersion() == 4) {
            caps << "bluetooth-5";  // RPi4 has BT5 but not WiFi6
        }
    }

    if (hasInternetConnection()) {
        caps << "internet";
    }

    if (hasExternalDisplay()) {
        caps << "multi-display";
    }

    return caps;
}
```

### 6. Rollout Strategy

**Deterministic Cohort Assignment** (user always gets same rollout state):

```cpp
bool FeatureFlagService::evaluateRollout(const QString& featureName) const {
    const auto& feature = featureFlags_["features"][featureName.toStdString()];
    int rolloutPct = feature["rolloutPercentage"].asInt();

    if (rolloutPct == 100) return true;
    if (rolloutPct == 0) return false;

    // Hash device ID + feature name → deterministic cohort (0-99)
    QByteArray input = (deviceId_ + featureName).toUtf8();
    QByteArray hash = QCryptographicHash::hash(input, QCryptographicHash::Sha256);
    int cohort = (hash[0] << 8 | hash[1]) % 100;

    return cohort < rolloutPct;
}
```

### 7. Feature Registry Examples

```
Bronze
- Android Auto (USB)
- Basic Bluetooth
- WiFi connectivity
- Light/dark theme

Silver
- Wireless Android Auto projection
- Advanced Bluetooth controls
- Theme customisation
- Profile management

Gold
- Custom radio extensions
- Bluetooth Mesh networking
- Advanced vehicle controls
- Extension store access

Platinum
- Advanced diagnostics (OBD-II)
- Multiple display support
- Server-side synchronisation
- Priority support, early access to beta features
```

### 8. Configuration Examples

**For licensing server validation (future)**:
```json
{
  "licensing": {
    "serverValidation": {
      "enabled": false,
      "endpoint": "https://licensing.opencardev.org/validate",
      "validateInterval": 604800,
      "offlineGracePeriod": 30
    }
  }
}
```

**For testing/development**:
```json
{
  "debug": {
    "forceFeatureFlags": {
      "advanced-diagnostics": true,
      "server-side-sync": false
    },
    "forceTier": "platinum"
  }
}
```

## Implementation Steps

### Phase 1: Foundation
1. Create FeatureFlagService with feature loading and tier evaluation
2. Create `config/features.json` with initial feature set
3. Update `config/crankshaft.json` with licensing section
4. Wire FeatureFlagService into application startup

### Phase 2: Core Integration
1. Update ServiceManager to respect feature flags
2. Add hardware capability detection to HostHAL
3. Implement rollout evaluation (deterministic hashing)
4. Add WebSocket API endpoints for feature queries

### Phase 3: UI Integration
1. Create WebSocket message handlers in UI
2. Add QML singleton for feature state (`ui/qml/models/FeaturesModel.qml`)
3. Update menus, settings screens to hide disabled features
4. Add feature gating to navigation/buttons

### Phase 4: Extension System
1. Update extension loader to check feature requirements
2. Add feature dependency validation
3. Implement extension rollback if feature becomes disabled

### Phase 5: Testing & Documentation
1. Add unit tests for FeatureFlagService
2. Create feature flag documentation
3. Document licensing tier assignments
4. Add testing guides for rollout scenarios

## Key Design Decisions

### 1. JSON-Based (Not Database)
- Aligns with existing config system
- Human-readable for testing/debugging
- Easy to version control and review
- Simple to parse at startup

### 2. Deterministic Rollout
- Same device always gets same rollout cohort
- Hash-based (sha256) ensures consistency
- No server sync needed for rollout decisions
- Allow override via `debug.forceFeatureFlags` for QA

### 3. Tier as Enum (Not String)
- Type-safe comparisons (`tier >= Gold`)
- Easier to extend with new tiers
- Prevents typos in code

### 4. Silent Features by Default
- New features added to highest tier (Platinum) with 0% rollout
- Gradually increase rollout percentage as feature matures
- Gradually lower tier (Platinum → Gold → Silver → Bronze)
- Users on lower tiers never see incomplete features

### 5. Hardware Gates Independent of License
- Feature can be unavailable even for Platinum if hardware lacks capability
- `minTier: "bronze"` + `requires: ["wifi6"]` = only Platinum on RPi5, never Bronze on RPi4
- Reflects real-world constraints

## Future Enhancements

### Server-Side Licensing
- Optional server validation endpoint for license authenticity
- Server can push feature flag overrides
- Enables dynamic A/B testing from backend
- Requires internet; graceful fallback to local config

### Telemetry
- Track which tier users are in (anonymised)
- Monitor rollout adoption rates
- Detect feature compatibility issues per tier

### Feature Store/Marketplace
- Gold+ users can browse/enable community extensions
- Extensions published with minimum tier requirement
- Server validates and delivers extension code

### Dynamic Tier Upgrade
- UI to view current tier, upgrade path, pricing
- Integration with payment processor
- Automatic tier upgrade after payment

### Feature Deprecation
- Mark features as deprecated
- Show warnings to users before removal
- Gradual phase-out path

## Example Usage

### In C++ (Core)
```cpp
auto& flags = FeatureFlagService::instance();

if (flags.isFeatureEnabled("android-auto")) {
    androidAutoService = std::make_unique<AndroidAutoService>();
}

if (!flags.validateFeatureDependencies("wireless-projection")) {
    qWarning() << "Cannot enable wireless projection (missing dependencies)";
}
```

### In QML (UI)
```qml
import Crankshaft 1.0

Rectangle {
    visible: FeaturesModel.isFeatureEnabled("advanced-diagnostics")

    Button {
        enabled: FeaturesModel.isFeatureEnabled("custom-radio-extensions")
        onClicked: stack.push("radioExtensionsPage")
    }
}
```

### WebSocket (Extensions)
```javascript
wsClient.subscribe("features/#");

wsClient.on("features/enabled", (data) => {
    console.log("Enabled features:", data.features);
});

// Query single feature
wsClient.publish("features/check", { name: "advanced-diagnostics" });
```

## Testing Strategy

### Unit Tests
- Feature flag loading and parsing
- Tier comparisons and logic
- Hardware capability evaluation
- Rollout cohort calculation (consistency)
- Dependency validation

### Integration Tests
- ServiceManager respects feature flags
- WebSocket API returns correct feature state
- UI updates when feature flags change
- Feature-gated extensions don't load

### Manual QA
- Test each tier's visible features on RPi4, RPi5, mock HAL
- Verify rollout cohorts are consistent (same device always sees same rollout)
- Test offline scenarios (server validation, grace period)
- Verify no feature "leakage" (Platinum-only feature visible on Bronze)

## Security Considerations

### 1. No Client-Side Override
- Feature flags downloaded to client, but enforced server-side
- UI hides features; core skips feature logic entirely
- Extensions cannot bypass tier checks

### 2. Hardware Fingerprinting
- MAC addresses (network interfaces) hashed to device ID
- Prevents spoofing; consistent across reboots
- Shared with licensing server (if enabled)

### 3. License Key Validation
- Placeholder for future server-side validation
- Offline mode uses last-known-good license state
- Grace period allows temporary offline use

### 4. Audit Logging
- Log all feature flag evaluations (especially denials)
- Log tier changes
- Log license validation success/failure
- Helps diagnose support issues

## Backward Compatibility

### Migration Path
1. Assign all existing features to "Bronze" tier with 100% rollout
2. No changes to existing installations
3. New features default to "Platinum" with 0% rollout
4. As features stabilise, gradually lower tier and increase rollout

### Configuration Evolution
- `crankshaft.json` version field for migration scripts
- Support multiple feature flag schema versions
- Graceful degradation if feature flag file missing
