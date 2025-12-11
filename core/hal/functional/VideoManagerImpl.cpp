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

#include "../multimedia/VideoManager.h"

// Placeholder implementation of VideoManager

class VideoManagerImpl : public VideoManager {
 public:
  explicit VideoManagerImpl(QObject* parent = nullptr) : VideoManager(parent) {}

  bool initialise() override {
    qDebug() << "[Video] Initialising VideoManager";
    // TODO: Initialise GStreamer video subsystem and display detection
    return true;
  }

  void deinitialise() override {
    qDebug() << "[Video] Deinitialising VideoManager";
  }

  QVector<QString> getDisplays() const override {
    return {"Primary"};
  }

  QVector<QString> getOutputDevices() const override {
    return {"HDMI-1", "DSI-1"};
  }

  VideoOutputDevicePtr getOutputDevice(const QString& name) const override {
    return nullptr;
  }

  VideoInputDevicePtr getInputDevice(const QString& name) const override {
    return nullptr;
  }

  QString getPrimaryDisplay() const override {
    return "Primary";
  }

  bool setDisplayMode(DisplayMode mode) override {
    return true;
  }

  DisplayMode getDisplayMode() const override {
    return DisplayMode::FULLSCREEN;
  }

  bool setResolution(const QSize& resolution) override {
    emit resolutionChanged(resolution);
    return true;
  }

  QSize getResolution() const override {
    return {1024, 600};
  }

  bool setRefreshRate(int hz) override {
    emit refreshRateChanged(hz);
    return true;
  }

  int getRefreshRate() const override {
    return 60;
  }

  void setBrightness(int percent) override {
    emit brightnessChanged(percent);
  }

  int getBrightness() const override {
    return 80;
  }

  void setContrast(int percent) override {
    emit contrastChanged(percent);
  }

  int getContrast() const override {
    return 50;
  }

  void setNightModeEnabled(bool enabled) override {
    emit nightModeChanged(enabled);
  }

  bool isNightModeEnabled() const override {
    return false;
  }

  void setColourTemperature(int kelvin) override {}

  int getColourTemperature() const override {
    return 6500;
  }
};
