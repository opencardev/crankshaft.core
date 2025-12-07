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

#include <QString>
#include <QObject>
#include <QSize>
#include <memory>

/**
 * @brief Base class for video device abstraction
 *
 * Provides interface for video output and decoding.
 * Implementations may use GStreamer, OpenGL, Vulkan, etc.
 */
class VideoDevice : public QObject {
  Q_OBJECT

 public:
  enum class VideoFormat {
    H264,
    H265,
    VP8,
    VP9,
    AV1,
  };

  enum class PixelFormat {
    YUV420,
    YUV422,
    RGBA,
    RGB24,
  };

  struct VideoConfig {
    VideoFormat format = VideoFormat::H264;
    PixelFormat pixel_format = PixelFormat::YUV420;
    QSize resolution{1024, 600};
    int framerate = 30;
    int bitrate = 5000;  // kbps
  };

  explicit VideoDevice(QObject* parent = nullptr) : QObject(parent) {}
  virtual ~VideoDevice() = default;

  /**
   * @brief Initialise the video device
   * @param config Video configuration
   * @return true if initialisation successful
   */
  virtual bool initialise(const VideoConfig& config) = 0;

  /**
   * @brief Deinitialise the video device
   */
  virtual void deinitialise() = 0;

  /**
   * @brief Check if device is initialised
   */
  virtual bool isInitialised() const = 0;

  /**
   * @brief Get current video configuration
   */
  virtual VideoConfig getConfig() const = 0;

  /**
   * @brief Get device name/identifier
   */
  virtual QString getDeviceName() const = 0;

  /**
   * @brief Get supported video formats
   */
  virtual QVector<VideoFormat> getSupportedFormats() const = 0;

  /**
   * @brief Get supported resolutions
   */
  virtual QVector<QSize> getSupportedResolutions() const = 0;

 signals:
  /**
   * @brief Emitted when device error occurs
   */
  void errorOccurred(const QString& error);

  /**
   * @brief Emitted when device state changes
   */
  void stateChanged(const QString& state);

  /**
   * @brief Emitted when resolution changes
   */
  void resolutionChanged(const QSize& resolution);
};

/**
 * @brief Output video device (display renderer)
 */
class VideoOutputDevice : public VideoDevice {
  Q_OBJECT

 public:
  explicit VideoOutputDevice(QObject* parent = nullptr)
      : VideoDevice(parent) {}
  ~VideoOutputDevice() override = default;

  /**
   * @brief Write video frame data
   * @param data Frame buffer
   * @param size Size of buffer in bytes
   * @return true if write successful
   */
  virtual bool writeFrame(const uint8_t* data, int size) = 0;

  /**
   * @brief Set brightness (0-100)
   */
  virtual void setBrightness(int percent) = 0;

  /**
   * @brief Get current brightness
   */
  virtual int getBrightness() const = 0;

  /**
   * @brief Set contrast (0-100)
   */
  virtual void setContrast(int percent) = 0;

  /**
   * @brief Get current contrast
   */
  virtual int getContrast() const = 0;

  /**
   * @brief Enable/disable vsync
   */
  virtual void setVSyncEnabled(bool enabled) = 0;

  /**
   * @brief Get vsync state
   */
  virtual bool isVSyncEnabled() const = 0;

 signals:
  /**
   * @brief Emitted when frame is rendered
   */
  void frameRendered();

  /**
   * @brief Emitted when brightness changes
   */
  void brightnessChanged(int percent);

  /**
   * @brief Emitted when contrast changes
   */
  void contrastChanged(int percent);
};

/**
 * @brief Input video device (camera)
 */
class VideoInputDevice : public VideoDevice {
  Q_OBJECT

 public:
  explicit VideoInputDevice(QObject* parent = nullptr)
      : VideoDevice(parent) {}
  ~VideoInputDevice() override = default;

  /**
   * @brief Read video frame data
   * @param buffer Destination buffer
   * @param size Maximum bytes to read
   * @return Bytes read
   */
  virtual int readFrame(uint8_t* buffer, int size) = 0;

  /**
   * @brief Set focus mode
   */
  virtual void setFocusMode(const QString& mode) = 0;

  /**
   * @brief Get focus mode
   */
  virtual QString getFocusMode() const = 0;

  /**
   * @brief Set zoom level (1.0 = no zoom, > 1.0 = zoom in)
   */
  virtual void setZoom(float level) = 0;

  /**
   * @brief Get current zoom level
   */
  virtual float getZoom() const = 0;
};

using VideoOutputDevicePtr = std::shared_ptr<VideoOutputDevice>;
using VideoInputDevicePtr = std::shared_ptr<VideoInputDevice>;
