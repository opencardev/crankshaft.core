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

#include "DrivingModeService.h"

#include <QDebug>
#include <algorithm>

DrivingModeService::DrivingModeService(QObject *parent)
    : QObject(parent), m_isDrivingMode(false), m_vehicleSpeedMph(0.0f), m_isRestricted(false) {}

void DrivingModeService::onVehicleSpeedUpdated(float speedMph) {
  // Update speed
  if (m_vehicleSpeedMph != speedMph) {
    m_vehicleSpeedMph = speedMph;
    emit vehicleSpeedChanged(speedMph);
  }

  // Update driving mode based on speed threshold
  bool wasDriving = m_isDrivingMode;
  m_isDrivingMode = speedMph > m_speedThresholdMph;

  if (wasDriving != m_isDrivingMode) {
    emit drivingModeChanged(m_isDrivingMode);
    updateRestrictionState();

    if (m_isDrivingMode) {
      qInfo() << "DrivingModeService: Driving mode activated at" << speedMph << "mph";
    } else {
      qInfo() << "DrivingModeService: Driving mode deactivated";
    }
  }

  // Update restriction state if speed affects restrictions
  updateRestrictionState();
}

void DrivingModeService::setDrivingMode(bool enabled) {
  if (m_isDrivingMode != enabled) {
    m_isDrivingMode = enabled;
    emit drivingModeChanged(enabled);
    updateRestrictionState();

    qInfo() << "DrivingModeService: Driving mode manually set to" << enabled;
  }
}

void DrivingModeService::resetDrivingMode() {
  if (m_isDrivingMode || m_vehicleSpeedMph > 0.0f) {
    m_isDrivingMode = false;
    m_vehicleSpeedMph = 0.0f;
    emit drivingModeChanged(false);
    emit vehicleSpeedChanged(0.0f);
    updateRestrictionState();

    qInfo() << "DrivingModeService: Driving mode reset";
  }
}

void DrivingModeService::setSpeedThresholdMph(float mph) {
  if (m_speedThresholdMph != mph) {
    m_speedThresholdMph = mph;
    emit settingsChanged();

    // Re-evaluate driving mode with new threshold
    onVehicleSpeedUpdated(m_vehicleSpeedMph);

    qInfo() << "DrivingModeService: Speed threshold set to" << mph << "mph";
  }
}

void DrivingModeService::setStrictMode(bool strict) {
  if (m_strictMode != strict) {
    m_strictMode = strict;
    emit settingsChanged();
    updateRestrictionState();

    qInfo() << "DrivingModeService: Strict mode" << (strict ? "enabled" : "disabled");
  }
}

bool DrivingModeService::isActionAllowed(const QString &action) {
  // Always allow safe actions
  if (isActionInList(action, SAFE_ACTIONS, 9)) {
    return true;
  }

  // In driving mode, restrict dangerous actions
  if (m_isDrivingMode) {
    if (isActionInList(action, RESTRICTED_ACTIONS, 4)) {
      emit restrictedActionAttempted(action);
      return false;
    }
  }

  return true;
}

void DrivingModeService::requestRestrictionExemption(const QString &reason) {
  emit exemptionRequested(reason);
  qWarning() << "DrivingModeService: Exemption requested:" << reason;
}

void DrivingModeService::acknowledgeRestriction() {
  m_isRestricted = false;
  emit restrictionChanged(false);
}

QString DrivingModeService::getRestrictionMessage() const {
  if (!m_isDrivingMode) {
    return QString();
  }

  if (m_strictMode) {
    return "You are driving. Complex interactions are disabled for your safety. "
           "Use voice control or predefined buttons only.";
  } else {
    return "Driving mode active. Keyboard input is restricted. Use touch controls "
           "or voice commands.";
  }
}

QStringList DrivingModeService::getRestrictedActions() const {
  QStringList restricted;
  for (size_t i = 0; i < 4; ++i) {
    restricted << QString::fromUtf8(RESTRICTED_ACTIONS[i]);
  }
  return restricted;
}

void DrivingModeService::updateRestrictionState() {
  bool wasRestricted = m_isRestricted;
  m_isRestricted = m_isDrivingMode;

  if (m_isRestricted) {
    m_restrictionReason = getRestrictionMessage();
  } else {
    m_restrictionReason = QString();
  }

  if (wasRestricted != m_isRestricted) {
    emit restrictionChanged(m_isRestricted);
  }
}

bool DrivingModeService::isActionInList(const QString &action, const char *const *list,
                                        size_t count) const {
  for (size_t i = 0; i < count; ++i) {
    if (action == QString::fromUtf8(list[i])) {
      return true;
    }
  }
  return false;
}
