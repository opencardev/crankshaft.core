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

#pragma once

#include "AudioDevice.h"
#include <QString>
#include <QObject>
#include <QVector>
#include <memory>

/**
 * @brief Audio system manager
 *
 * Manages audio devices, routing, mixing, and global audio settings.
 * Handles multiple audio streams and device enumeration.
 */
class AudioManager : public QObject {
  Q_OBJECT

 public:
  enum class AudioRoute {
    SPEAKER,
    HEADPHONE,
    BLUETOOTH,
    USB,
    DEFAULT,
  };

  explicit AudioManager(QObject* parent = nullptr);
  ~AudioManager() override;

  /**
   * @brief Initialise the audio manager
   * @return true if initialisation successful
   */
  virtual bool initialise() = 0;

  /**
   * @brief Deinitialise the audio manager
   */
  virtual void deinitialise() = 0;

  /**
   * @brief Enumerate available output devices
   */
  virtual QVector<QString> getOutputDevices() const = 0;

  /**
   * @brief Enumerate available input devices
   */
  virtual QVector<QString> getInputDevices() const = 0;

  /**
   * @brief Get output device by name
   */
  virtual AudioOutputDevicePtr getOutputDevice(
      const QString& name) const = 0;

  /**
   * @brief Get input device by name
   */
  virtual AudioInputDevicePtr getInputDevice(const QString& name) const = 0;

  /**
   * @brief Get default output device
   */
  virtual AudioOutputDevicePtr getDefaultOutputDevice() const = 0;

  /**
   * @brief Get default input device
   */
  virtual AudioInputDevicePtr getDefaultInputDevice() const = 0;

  /**
   * @brief Set audio route
   */
  virtual bool setAudioRoute(AudioRoute route) = 0;

  /**
   * @brief Get current audio route
   */
  virtual AudioRoute getAudioRoute() const = 0;

  /**
   * @brief Set master volume (0-100)
   */
  virtual void setMasterVolume(int percent) = 0;

  /**
   * @brief Get master volume
   */
  virtual int getMasterVolume() const = 0;

  /**
   * @brief Set stream volume by category
   */
  virtual void setStreamVolume(const QString& stream_type, int percent) = 0;

  /**
   * @brief Get stream volume
   */
  virtual int getStreamVolume(const QString& stream_type) const = 0;

  /**
   * @brief Mute/unmute all audio
   */
  virtual void setMuted(bool muted) = 0;

  /**
   * @brief Check if audio is muted
   */
  virtual bool isMuted() const = 0;

 signals:
  /**
   * @brief Emitted when output device is connected/disconnected
   */
  void outputDeviceChanged(const QString& device_name, bool connected);

  /**
   * @brief Emitted when input device is connected/disconnected
   */
  void inputDeviceChanged(const QString& device_name, bool connected);

  /**
   * @brief Emitted when audio route changes
   */
  void audioRouteChanged(AudioRoute route);

  /**
   * @brief Emitted when master volume changes
   */
  void masterVolumeChanged(int percent);

  /**
   * @brief Emitted when stream volume changes
   */
  void streamVolumeChanged(const QString& stream_type, int percent);

  /**
   * @brief Emitted when mute state changes
   */
  void muteStateChanged(bool muted);

  /**
   * @brief Emitted when audio error occurs
   */
  void errorOccurred(const QString& error);
};

using AudioManagerPtr = std::shared_ptr<AudioManager>;
