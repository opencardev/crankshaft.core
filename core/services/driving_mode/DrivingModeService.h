/*
 * Project: Crankshaft
 * This file is part of Crankshaft project.
 * Copyright (C) 2025 OpenCarDev Team
 *
 *  Crankshaft is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Crankshaft is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Crankshaft. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DRIVING_MODE_SERVICE_H
#define DRIVING_MODE_SERVICE_H

#include <QObject>
#include <QString>

/**
 * @brief Service to detect driving state and enforce safety restrictions
 *
 * Monitors vehicle speed and other indicators to determine if the driver
 * is actively driving. When in driving mode, restricts complex UI interactions:
 * - Disables full keyboard input (only voice/predefined buttons)
 * - Restricts access to settings
 * - Limits configuration changes
 * - Focuses on safety-critical controls only
 *
 * Complies with Google Design for Driving guidelines:
 * - Glance time: <2-3 seconds for any interaction
 * - Large touch targets: 48dp+ when driving
 * - No text input while driving
 * - Haptic feedback for confirmations
 */
class DrivingModeService : public QObject {
  Q_OBJECT

  // Driving state and related properties
  Q_PROPERTY(bool isDrivingMode READ isDrivingMode NOTIFY drivingModeChanged)
  Q_PROPERTY(float vehicleSpeedMph READ vehicleSpeedMph NOTIFY vehicleSpeedChanged)
  Q_PROPERTY(bool isRestricted READ isRestricted NOTIFY restrictionChanged)
  Q_PROPERTY(QString restrictionReason READ restrictionReason NOTIFY restrictionChanged)

  // User preferences
  Q_PROPERTY(float speedThresholdMph READ speedThresholdMph WRITE setSpeedThresholdMph NOTIFY
                 settingsChanged)
  Q_PROPERTY(bool strictMode READ strictMode WRITE setStrictMode NOTIFY settingsChanged)

 public:
  explicit DrivingModeService(QObject *parent = nullptr);
  ~DrivingModeService() = default;

  // Getters
  bool isDrivingMode() const {
    return m_isDrivingMode;
  }
  float vehicleSpeedMph() const {
    return m_vehicleSpeedMph;
  }
  bool isRestricted() const {
    return m_isRestricted;
  }
  QString restrictionReason() const {
    return m_restrictionReason;
  }
  float speedThresholdMph() const {
    return m_speedThresholdMph;
  }
  bool strictMode() const {
    return m_strictMode;
  }

  // Setters
  void setSpeedThresholdMph(float mph);
  void setStrictMode(bool strict);

  // Public API: Check if specific action is allowed
  Q_INVOKABLE bool isActionAllowed(const QString &action);
  Q_INVOKABLE void requestRestrictionExemption(const QString &reason);
  Q_INVOKABLE void acknowledgeRestriction();

  // Public API: Get restriction state for UI components
  Q_INVOKABLE QString getRestrictionMessage() const;
  Q_INVOKABLE QStringList getRestrictedActions() const;

 public slots:
  // Slot to receive vehicle speed updates (from CAN bus, GPS, etc.)
  void onVehicleSpeedUpdated(float speedMph);

  // Slot to manually enable/disable driving mode
  void setDrivingMode(bool enabled);

  // Slot to reset driving mode
  void resetDrivingMode();

 signals:
  void drivingModeChanged(bool isDriving);
  void vehicleSpeedChanged(float speedMph);
  void restrictionChanged(bool isRestricted);
  void settingsChanged();
  void restrictedActionAttempted(const QString &action);
  void exemptionRequested(const QString &reason);

 private:
  // Driving state
  bool m_isDrivingMode = false;
  float m_vehicleSpeedMph = 0.0f;
  bool m_isRestricted = false;
  QString m_restrictionReason;

  // Settings
  float m_speedThresholdMph = 5.0f;  // Driving starts above 5 mph
  bool m_strictMode = false;         // When true, enforce stricter restrictions

  // Restricted actions
  static constexpr const char *RESTRICTED_ACTIONS[] = {
      "ui.fullKeyboard",       // Full text input keyboard
      "ui.settings.modify",    // Modifying settings while driving
      "ui.extension.install",  // Installing extensions
      "ui.system.configure",   // System configuration
  };

  // Safe actions (allowed while driving)
  static constexpr const char *SAFE_ACTIONS[] = {
      "media.play",        "media.pause",  "media.skip",   "media.volume",       "navigation.start",
      "navigation.cancel", "phone.answer", "phone.reject", "phone.voiceControl",
  };

  void updateRestrictionState();
  bool isActionInList(const QString &action, const char *const *list, size_t count) const;
};

#endif  // DRIVING_MODE_SERVICE_H
