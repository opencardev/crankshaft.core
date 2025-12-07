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

#include "VideoDevice.h"
#include <QString>
#include <QObject>
#include <QVector>
#include <QSize>
#include <memory>

/**
 * @brief Video system manager
 *
 * Manages video devices, rendering, and global video settings.
 * Handles multiple video streams and display enumeration.
 */
class VideoManager : public QObject {
  Q_OBJECT

 public:
  enum class DisplayMode {
    FULLSCREEN,
    WINDOWED,
    EXTENDED,
    MIRROR,
  };

  explicit VideoManager(QObject* parent = nullptr);
  ~VideoManager() override;

  /**
   * @brief Initialise the video manager
   * @return true if initialisation successful
   */
  virtual bool initialise() = 0;

  /**
   * @brief Deinitialise the video manager
   */
  virtual void deinitialise() = 0;

  /**
   * @brief Enumerate available displays
   */
  virtual QVector<QString> getDisplays() const = 0;

  /**
   * @brief Enumerate available output devices
   */
  virtual QVector<QString> getOutputDevices() const = 0;

  /**
   * @brief Get output device by name
   */
  virtual VideoOutputDevicePtr getOutputDevice(
      const QString& name) const = 0;

  /**
   * @brief Get input device by name
   */
  virtual VideoInputDevicePtr getInputDevice(const QString& name) const = 0;

  /**
   * @brief Get primary display
   */
  virtual QString getPrimaryDisplay() const = 0;

  /**
   * @brief Set display mode
   */
  virtual bool setDisplayMode(DisplayMode mode) = 0;

  /**
   * @brief Get current display mode
   */
  virtual DisplayMode getDisplayMode() const = 0;

  /**
   * @brief Set display resolution
   */
  virtual bool setResolution(const QSize& resolution) = 0;

  /**
   * @brief Get current display resolution
   */
  virtual QSize getResolution() const = 0;

  /**
   * @brief Set refresh rate
   */
  virtual bool setRefreshRate(int hz) = 0;

  /**
   * @brief Get current refresh rate
   */
  virtual int getRefreshRate() const = 0;

  /**
   * @brief Set global brightness (0-100)
   */
  virtual void setBrightness(int percent) = 0;

  /**
   * @brief Get global brightness
   */
  virtual int getBrightness() const = 0;

  /**
   * @brief Set global contrast (0-100)
   */
  virtual void setContrast(int percent) = 0;

  /**
   * @brief Get global contrast
   */
  virtual int getContrast() const = 0;

  /**
   * @brief Enable/disable night mode
   */
  virtual void setNightModeEnabled(bool enabled) = 0;

  /**
   * @brief Check if night mode is enabled
   */
  virtual bool isNightModeEnabled() const = 0;

  /**
   * @brief Set colour temperature for night mode (2700K-6500K)
   */
  virtual void setColourTemperature(int kelvin) = 0;

  /**
   * @brief Get colour temperature
   */
  virtual int getColourTemperature() const = 0;

 signals:
  /**
   * @brief Emitted when display is connected/disconnected
   */
  void displayChanged(const QString& display, bool connected);

  /**
   * @brief Emitted when display mode changes
   */
  void displayModeChanged(DisplayMode mode);

  /**
   * @brief Emitted when resolution changes
   */
  void resolutionChanged(const QSize& resolution);

  /**
   * @brief Emitted when refresh rate changes
   */
  void refreshRateChanged(int hz);

  /**
   * @brief Emitted when brightness changes
   */
  void brightnessChanged(int percent);

  /**
   * @brief Emitted when contrast changes
   */
  void contrastChanged(int percent);

  /**
   * @brief Emitted when night mode state changes
   */
  void nightModeChanged(bool enabled);

  /**
   * @brief Emitted when video error occurs
   */
  void errorOccurred(const QString& error);
};

using VideoManagerPtr = std::shared_ptr<VideoManager>;
