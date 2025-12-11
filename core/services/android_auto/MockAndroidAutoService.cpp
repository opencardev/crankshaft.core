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

#include "MockAndroidAutoService.h"

#include <QJsonObject>
#include <QPainter>
#include <QRandomGenerator>

MockAndroidAutoService::MockAndroidAutoService(QObject* parent) : AndroidAutoService(parent) {
  // Initialize mock device info
  m_device.serialNumber = "MOCK_AA_DEVICE_001";
  m_device.manufacturer = "Google";
  m_device.model = "Pixel 7 Pro";
  m_device.androidVersion = "14.0";
  m_device.connected = false;
  m_device.projectionMode = ProjectionMode::PROJECTION;

  // Create timers
  m_connectionTimer = new QTimer(this);
  m_videoTimer = new QTimer(this);
  m_audioTimer = new QTimer(this);
  m_statsTimer = new QTimer(this);

  connect(m_connectionTimer, &QTimer::timeout, this, &MockAndroidAutoService::onConnectionTimer);
  connect(m_videoTimer, &QTimer::timeout, this, &MockAndroidAutoService::onVideoFrameTimer);
  connect(m_audioTimer, &QTimer::timeout, this, &MockAndroidAutoService::onAudioDataTimer);
  connect(m_statsTimer, &QTimer::timeout, this, &MockAndroidAutoService::onStatsTimer);
}

MockAndroidAutoService::~MockAndroidAutoService() {
  deinitialise();
}

bool MockAndroidAutoService::initialise() {
  if (m_state != ConnectionState::DISCONNECTED) {
    return false;
  }

  transitionToState(ConnectionState::SEARCHING);
  return true;
}

void MockAndroidAutoService::deinitialise() {
  stopSearching();
  if (isConnected()) {
    disconnect();
  }
  transitionToState(ConnectionState::DISCONNECTED);
}

bool MockAndroidAutoService::startSearching() {
  if (m_state != ConnectionState::DISCONNECTED && m_state != ConnectionState::SEARCHING) {
    return false;
  }

  transitionToState(ConnectionState::SEARCHING);

  // Simulate finding a device after a short delay
  QTimer::singleShot(500, this, [this]() {
    if (m_state == ConnectionState::SEARCHING) {
      emit deviceFound(m_device);
    }
  });

  return true;
}

void MockAndroidAutoService::stopSearching() {
  if (m_state == ConnectionState::SEARCHING) {
    transitionToState(ConnectionState::DISCONNECTED);
  }
}

bool MockAndroidAutoService::connectToDevice(const QString& serial) {
  if (m_state != ConnectionState::SEARCHING && m_state != ConnectionState::DISCONNECTED) {
    return false;
  }

  if (serial != m_device.serialNumber) {
    emit errorOccurred("Unknown device: " + serial);
    return false;
  }

  // Start connection state machine
  m_connectionStep = 0;
  transitionToState(ConnectionState::CONNECTING);
  m_connectionTimer->start(m_connectionDelayMs / 4);

  return true;
}

bool MockAndroidAutoService::disconnect() {
  if (m_state == ConnectionState::DISCONNECTED) {
    return false;
  }

  m_connectionTimer->stop();
  m_videoTimer->stop();
  m_audioTimer->stop();
  m_statsTimer->stop();

  transitionToState(ConnectionState::DISCONNECTING);

  // Simulate disconnection delay
  QTimer::singleShot(200, this, [this]() {
    m_device.connected = false;
    transitionToState(ConnectionState::DISCONNECTED);
    emit disconnected();
  });

  return true;
}

bool MockAndroidAutoService::setDisplayResolution(const QSize& resolution) {
  if (resolution.width() <= 0 || resolution.height() <= 0) {
    return false;
  }

  m_resolution = resolution;
  return true;
}

bool MockAndroidAutoService::setFramerate(int fps) {
  if (fps <= 0 || fps > 60) {
    return false;
  }

  m_fps = fps;

  // Update video timer if generating video
  if (m_generateVideo && m_videoTimer->isActive()) {
    m_videoTimer->setInterval(1000 / m_fps);
  }

  return true;
}

bool MockAndroidAutoService::sendTouchInput(int x, int y, int action) {
  if (!isConnected()) {
    return false;
  }

  // Just log the touch input in mock mode
  // In real implementation, this would send to AASDK
  return true;
}

bool MockAndroidAutoService::sendKeyInput(int key_code, int action) {
  if (!isConnected()) {
    return false;
  }

  // Just log the key input in mock mode
  return true;
}

bool MockAndroidAutoService::requestAudioFocus() {
  if (!isConnected()) {
    return false;
  }

  return true;
}

bool MockAndroidAutoService::abandonAudioFocus() {
  if (!isConnected()) {
    return false;
  }

  return true;
}

bool MockAndroidAutoService::setAudioEnabled(bool enabled) {
  m_audioEnabled = enabled;

  if (isConnected()) {
    if (enabled && !m_audioTimer->isActive() && m_generateAudio) {
      m_audioTimer->start(20);  // 50 Hz audio updates
    } else if (!enabled && m_audioTimer->isActive()) {
      m_audioTimer->stop();
    }
  }

  return true;
}

QJsonObject MockAndroidAutoService::getAudioConfig() const {
  QJsonObject config;
  config["sampleRate"] = 48000;
  config["channels"] = 2;
  config["bitsPerSample"] = 16;
  config["codec"] = "PCM";
  return config;
}

void MockAndroidAutoService::setGenerateTestVideo(bool enabled) {
  m_generateVideo = enabled;

  if (enabled && isConnected() && !m_videoTimer->isActive()) {
    m_videoTimer->start(1000 / m_fps);
    m_frameCount = 0;
  } else if (!enabled && m_videoTimer->isActive()) {
    m_videoTimer->stop();
  }
}

void MockAndroidAutoService::setGenerateTestAudio(bool enabled) {
  m_generateAudio = enabled;

  if (enabled && isConnected() && m_audioEnabled && !m_audioTimer->isActive()) {
    m_audioTimer->start(20);  // 50 Hz
  } else if (!enabled && m_audioTimer->isActive()) {
    m_audioTimer->stop();
  }
}

void MockAndroidAutoService::onConnectionTimer() {
  // Connection state machine
  switch (m_connectionStep) {
    case 0:  // Connecting
      transitionToState(ConnectionState::AUTHENTICATING);
      m_connectionStep++;
      break;

    case 1:  // Authenticating
      if (m_simulateErrors && QRandomGenerator::global()->bounded(100) < 10) {
        // 10% chance of auth failure
        m_connectionTimer->stop();
        emit errorOccurred("Authentication failed");
        transitionToState(ConnectionState::ERROR);
        return;
      }
      transitionToState(ConnectionState::SECURING);
      m_connectionStep++;
      break;

    case 2:  // Securing
      transitionToState(ConnectionState::CONNECTED);
      m_connectionTimer->stop();
      m_device.connected = true;
      m_elapsedTimer.start();

      // Start video/audio if enabled
      if (m_generateVideo) {
        m_videoTimer->start(1000 / m_fps);
      }
      if (m_generateAudio && m_audioEnabled) {
        m_audioTimer->start(20);
      }

      // Start stats timer
      m_statsTimer->start(1000);

      emit connected(m_device);
      break;
  }
}

void MockAndroidAutoService::onVideoFrameTimer() {
  if (!isConnected()) {
    return;
  }

  generateTestVideoFrame();
  m_frameCount++;

  // Simulate occasional frame drops
  if (m_simulateErrors && QRandomGenerator::global()->bounded(100) < 2) {
    m_droppedFrames++;
  }
}

void MockAndroidAutoService::onAudioDataTimer() {
  if (!isConnected() || !m_audioEnabled) {
    return;
  }

  generateTestAudioData();
}

void MockAndroidAutoService::onStatsTimer() {
  if (!isConnected()) {
    return;
  }

  // Calculate latency (simulated)
  m_latency = 10 + QRandomGenerator::global()->bounded(20);

  emit statsUpdated(m_fps, m_latency, m_droppedFrames);
}

void MockAndroidAutoService::transitionToState(ConnectionState newState) {
  if (m_state == newState) {
    return;
  }

  m_state = newState;
  emit connectionStateChanged(newState);
}

void MockAndroidAutoService::generateTestVideoFrame() {
  // Create a simple test pattern
  QImage image = createTestPattern(m_frameCount);

  // Convert to RGBA buffer
  QImage rgba = image.convertToFormat(QImage::Format_RGBA8888);
  const uint8_t* data = rgba.constBits();
  int size = rgba.sizeInBytes();

  emit videoFrameReady(m_resolution.width(), m_resolution.height(), data, size);
}

void MockAndroidAutoService::generateTestAudioData() {
  // Generate simple sine wave test tone (440 Hz)
  static const int sampleRate = 48000;
  static const int samplesPerFrame = sampleRate / 50;  // 20ms
  static const int bytesPerSample = 4;                 // 16-bit stereo
  static double phase = 0.0;

  QByteArray audioData(samplesPerFrame * bytesPerSample, 0);
  qint16* samples = reinterpret_cast<qint16*>(audioData.data());

  const double frequency = 440.0;   // A4 note
  const double amplitude = 8192.0;  // -12 dB

  for (int i = 0; i < samplesPerFrame; i++) {
    qint16 sample = static_cast<qint16>(amplitude * std::sin(phase));
    samples[i * 2] = sample;      // Left
    samples[i * 2 + 1] = sample;  // Right

    phase += 2.0 * M_PI * frequency / sampleRate;
    if (phase >= 2.0 * M_PI) {
      phase -= 2.0 * M_PI;
    }
  }

  emit audioDataReady(audioData);
}

QImage MockAndroidAutoService::createTestPattern(int frameNumber) {
  QImage image(m_resolution, QImage::Format_RGB32);
  QPainter painter(&image);

  // Animated gradient background
  int offset = (frameNumber * 2) % m_resolution.width();
  QLinearGradient gradient(offset, 0, m_resolution.width() + offset, m_resolution.height());
  gradient.setColorAt(0, QColor(20, 20, 40));
  gradient.setColorAt(0.5, QColor(40, 40, 80));
  gradient.setColorAt(1, QColor(20, 20, 40));
  painter.fillRect(image.rect(), gradient);

  // Draw grid
  painter.setPen(QColor(60, 60, 100));
  for (int x = 0; x < m_resolution.width(); x += 100) {
    painter.drawLine(x, 0, x, m_resolution.height());
  }
  for (int y = 0; y < m_resolution.height(); y += 100) {
    painter.drawLine(0, y, m_resolution.width(), y);
  }

  // Draw title
  painter.setPen(Qt::white);
  painter.setFont(QFont("Arial", 48, QFont::Bold));
  painter.drawText(image.rect(), Qt::AlignCenter,
                   "Mock Android Auto\nFrame: " + QString::number(frameNumber));

  // Draw info text
  painter.setFont(QFont("Arial", 16));
  QString info = QString("Resolution: %1x%2 | FPS: %3 | Latency: %4ms")
                     .arg(m_resolution.width())
                     .arg(m_resolution.height())
                     .arg(m_fps)
                     .arg(m_latency);
  painter.drawText(10, m_resolution.height() - 20, info);

  // Draw animated circle
  int cx = m_resolution.width() / 2;
  int cy = m_resolution.height() / 2;
  int radius = 50 + 30 * std::sin(frameNumber * 0.05);
  painter.setBrush(QColor(100, 150, 255, 128));
  painter.setPen(Qt::NoPen);
  painter.drawEllipse(QPointF(cx, cy), radius, radius);

  return image;
}
