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

#include <QElapsedTimer>
#include <QImage>
#include <QTimer>

#include "AndroidAutoService.h"

/**
 * @brief Mock Android Auto service for testing/development
 *
 * Simulates an Android Auto connection without requiring actual hardware.
 * Useful for:
 *   - Development without physical device
 *   - Testing UI components
 *   - Simulating various connection states
 *   - Generating test video/audio data
 */
class MockAndroidAutoService : public AndroidAutoService {
  Q_OBJECT

 public:
  explicit MockAndroidAutoService(QObject* parent = nullptr);
  ~MockAndroidAutoService() override;

  bool initialise() override;
  void deinitialise() override;

  ConnectionState getConnectionState() const override {
    return m_state;
  }
  bool isConnected() const override {
    return m_state == ConnectionState::CONNECTED;
  }
  AndroidDevice getConnectedDevice() const override {
    return m_device;
  }

  bool startSearching() override;
  void stopSearching() override;
  bool connectToDevice(const QString& serial) override;
  bool disconnect() override;

  bool setDisplayResolution(const QSize& resolution) override;
  QSize getDisplayResolution() const override {
    return m_resolution;
  }

  bool setFramerate(int fps) override;
  int getFramerate() const override {
    return m_fps;
  }

  bool sendTouchInput(int x, int y, int action) override;
  bool sendKeyInput(int key_code, int action) override;

  bool requestAudioFocus() override;
  bool abandonAudioFocus() override;

  int getFrameDropCount() const override {
    return m_droppedFrames;
  }
  int getLatency() const override {
    return m_latency;
  }

  bool setAudioEnabled(bool enabled) override;
  QJsonObject getAudioConfig() const override;

  // Mock-specific configuration
  void setSimulateConnectionDelay(int ms) {
    m_connectionDelayMs = ms;
  }
  void setSimulateErrors(bool enabled) {
    m_simulateErrors = enabled;
  }
  void setGenerateTestVideo(bool enabled);
  void setGenerateTestAudio(bool enabled);

 private slots:
  void onConnectionTimer();
  void onVideoFrameTimer();
  void onAudioDataTimer();
  void onStatsTimer();

 private:
  void transitionToState(ConnectionState newState);
  void generateTestVideoFrame();
  void generateTestAudioData();
  QImage createTestPattern(int frameNumber);

  ConnectionState m_state{ConnectionState::DISCONNECTED};
  AndroidDevice m_device;
  QSize m_resolution{1024, 600};
  int m_fps{30};
  bool m_audioEnabled{true};

  // Mock behaviour settings
  int m_connectionDelayMs{1000};
  bool m_simulateErrors{false};
  bool m_generateVideo{false};
  bool m_generateAudio{false};

  // Statistics
  int m_droppedFrames{0};
  int m_latency{10};
  int m_frameCount{0};

  // Timers
  QTimer* m_connectionTimer{nullptr};
  QTimer* m_videoTimer{nullptr};
  QTimer* m_audioTimer{nullptr};
  QTimer* m_statsTimer{nullptr};
  QElapsedTimer m_elapsedTimer;

  // Connection state machine
  int m_connectionStep{0};
};
