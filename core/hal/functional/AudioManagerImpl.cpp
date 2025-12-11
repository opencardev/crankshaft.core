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

#include <QDebug>

#include "../multimedia/AudioManager.h"

// Placeholder implementation of AudioManager
// To be replaced with actual GStreamer implementation

class AudioManagerImpl : public AudioManager {
 public:
  explicit AudioManagerImpl(QObject* parent = nullptr) : AudioManager(parent) {}

  bool initialise() override {
    qDebug() << "[Audio] Initialising AudioManager";
    // TODO: Initialise GStreamer and ALSA subsystem
    return true;
  }

  void deinitialise() override {
    qDebug() << "[Audio] Deinitialising AudioManager";
    // TODO: Cleanup GStreamer resources
  }

  QVector<QString> getOutputDevices() const override {
    // TODO: Query ALSA for output devices
    return {"Speaker", "Headphone", "Bluetooth"};
  }

  QVector<QString> getInputDevices() const override {
    // TODO: Query ALSA for input devices
    return {"Microphone"};
  }

  AudioOutputDevicePtr getOutputDevice(const QString& name) const override {
    // TODO: Create device instance
    return nullptr;
  }

  AudioInputDevicePtr getInputDevice(const QString& name) const override {
    // TODO: Create device instance
    return nullptr;
  }

  AudioOutputDevicePtr getDefaultOutputDevice() const override {
    return getOutputDevice("Speaker");
  }

  AudioInputDevicePtr getDefaultInputDevice() const override {
    return getInputDevice("Microphone");
  }

  bool setAudioRoute(AudioRoute route) override {
    // TODO: Switch audio output device
    return true;
  }

  AudioRoute getAudioRoute() const override {
    return AudioRoute::SPEAKER;
  }

  void setMasterVolume(int percent) override {
    qDebug() << "[Audio] Setting master volume to" << percent;
    emit masterVolumeChanged(percent);
  }

  int getMasterVolume() const override {
    return 70;
  }

  void setStreamVolume(const QString& stream_type, int percent) override {
    qDebug() << "[Audio] Setting" << stream_type << "volume to" << percent;
    emit streamVolumeChanged(stream_type, percent);
  }

  int getStreamVolume(const QString& stream_type) const override {
    return 75;
  }

  void setMuted(bool muted) override {
    qDebug() << "[Audio] Setting mute to" << muted;
    emit muteStateChanged(muted);
  }

  bool isMuted() const override {
    return false;
  }
};
