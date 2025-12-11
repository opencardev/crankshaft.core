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

#include <QJsonObject>
#include <QObject>
#include <QSize>
#include <QString>
#include <memory>

// Forward declarations
class MediaPipeline;

/**
 * @brief Android Auto projection service
 *
 * Handles USB connection, protocol negotiation, and screen projection
 * of Android devices using AASDK library.
 */
class AndroidAutoService : public QObject {
  Q_OBJECT

 public:
  enum class ConnectionState {
    DISCONNECTED,
    SEARCHING,
    CONNECTING,
    AUTHENTICATING,
    SECURING,
    CONNECTED,
    DISCONNECTING,
    ERROR,
  };

  enum class ProjectionMode {
    PHONE_SCREEN,
    PROJECTION,
  };

  struct AndroidDevice {
    QString serialNumber;
    QString manufacturer;
    QString model;
    QString androidVersion;
    bool connected = false;
    ProjectionMode projectionMode = ProjectionMode::PROJECTION;
  };

  explicit AndroidAutoService(QObject* parent = nullptr);
  ~AndroidAutoService() override;

  /**
   * @brief Factory method to create Android Auto service
   * @param mediaPipeline Media pipeline for audio/video streaming
   * @param parent Parent QObject
   * @return Initialized service instance or nullptr on failure
   */
  static AndroidAutoService* create(MediaPipeline* mediaPipeline, QObject* parent = nullptr);

  /**
   * @brief Initialise the Android Auto service
   * @return true if initialisation successful
   */
  virtual bool initialise() = 0;

  /**
   * @brief Deinitialise the service
   */
  virtual void deinitialise() = 0;

  /**
   * @brief Get current connection state
   */
  virtual ConnectionState getConnectionState() const = 0;

  /**
   * @brief Check if a device is connected
   */
  virtual bool isConnected() const = 0;

  /**
   * @brief Get connected device info
   */
  virtual AndroidDevice getConnectedDevice() const = 0;

  /**
   * @brief Start searching for devices
   */
  virtual bool startSearching() = 0;

  /**
   * @brief Stop searching for devices
   */
  virtual void stopSearching() = 0;

  /**
   * @brief Connect to a device
   * @param serial Device serial number
   */
  virtual bool connectToDevice(const QString& serial) = 0;

  /**
   * @brief Disconnect from current device
   */
  virtual bool disconnect() = 0;

  /**
   * @brief Set projection display resolution
   */
  virtual bool setDisplayResolution(const QSize& resolution) = 0;

  /**
   * @brief Get projection display resolution
   */
  virtual QSize getDisplayResolution() const = 0;

  /**
   * @brief Set FPS for video projection
   */
  virtual bool setFramerate(int fps) = 0;

  /**
   * @brief Get current FPS
   */
  virtual int getFramerate() const = 0;

  /**
   * @brief Send touch input to device
   * @param x X coordinate
   * @param y Y coordinate
   * @param action Touch action (0=down, 1=up, 2=move)
   */
  virtual bool sendTouchInput(int x, int y, int action) = 0;

  /**
   * @brief Send key input to device
   * @param key_code Android key code
   * @param action Key action (0=down, 1=up)
   */
  virtual bool sendKeyInput(int key_code, int action) = 0;

  /**
   * @brief Request audio focus on device
   */
  virtual bool requestAudioFocus() = 0;

  /**
   * @brief Abandon audio focus on device
   */
  virtual bool abandonAudioFocus() = 0;

  /**
   * @brief Get video frames per second counter
   */
  virtual int getFrameDropCount() const = 0;

  /**
   * @brief Get latency in milliseconds
   */
  virtual int getLatency() const = 0;

  /**
   * @brief Enable/disable audio streaming
   */
  virtual bool setAudioEnabled(bool enabled) = 0;

  /**
   * @brief Get audio stream configuration
   */
  virtual QJsonObject getAudioConfig() const = 0;

 signals:
  /**
   * @brief Emitted when connection state changes
   */
  void connectionStateChanged(ConnectionState state);

  /**
   * @brief Emitted when device is found during search
   */
  void deviceFound(const AndroidDevice& device);

  /**
   * @brief Emitted when successfully connected
   */
  void connected(const AndroidDevice& device);

  /**
   * @brief Emitted when disconnected
   */
  void disconnected();

  /**
   * @brief Emitted when video frame is ready for rendering
   * @param width Frame width
   * @param height Frame height
   * @param data Frame buffer (RGBA)
   * @param size Buffer size in bytes
   */
  void videoFrameReady(int width, int height, const uint8_t* data, int size);

  /**
   * @brief Emitted when audio data is available
   */
  void audioDataReady(const QByteArray& data);

  /**
   * @brief Emitted when service error occurs
   */
  void errorOccurred(const QString& error);

  /**
   * @brief Emitted when service info changes (FPS, latency)
   */
  void statsUpdated(int fps, int latency, int dropped_frames);
};

using AndroidAutoServicePtr = std::shared_ptr<AndroidAutoService>;
