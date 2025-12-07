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

#include "../multimedia/AudioDevice.h"

// Placeholder implementations

class AudioOutputDeviceImpl : public AudioOutputDevice {
 public:
  bool initialise(const AudioConfig& config) override { return true; }
  void deinitialise() override {}
  bool isInitialised() const override { return true; }
  AudioConfig getConfig() const override { return {}; }
  QString getDeviceName() const override { return "Speaker"; }
  int write(const char* data, int size) override { return size; }
  void setVolume(int percent) override {}
  int getVolume() const override { return 70; }
  void setMuted(bool muted) override {}
  bool isMuted() const override { return false; }
};

class AudioInputDeviceImpl : public AudioInputDevice {
 public:
  bool initialise(const AudioConfig& config) override { return true; }
  void deinitialise() override {}
  bool isInitialised() const override { return true; }
  AudioConfig getConfig() const override { return {}; }
  QString getDeviceName() const override { return "Microphone"; }
  int read(char* buffer, int size) override { return 0; }
  void setGain(int percent) override {}
  int getGain() const override { return 50; }
};
