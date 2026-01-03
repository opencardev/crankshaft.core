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

#include <QAudioDevice>
#include <QByteArray>
#include <QMediaDevices>
#include <QObject>
#include <QString>
#include <QVariantMap>
#include <memory>

class MediaPipeline;
class Logger;

/**
 * @brief Audio stream roles for Android Auto
 *
 * Maps AASDK audio channels to vehicle audio zones
 */
enum class AAudioStreamRole {
  MEDIA,          ///< Media/music playback
  SYSTEM_AUDIO,   ///< Navigation and system sounds
  GUIDANCE,       ///< Voice guidance only
  PHONE_CALL,     ///< Phone call audio (future)
  UNKNOWN = -1
};

/**
 * @brief Audio Router for Android Auto media channels
 *
 * Handles routing of decoded PCM audio from AASDK audio channels
 * to vehicle audio outputs via PipeWire/PulseAudio stack.
 *
 * Audio Stream Types (from AASDK):
 * - AUDIO_STREAM_GUIDANCE (1): Navigation turn-by-turn directions
 * - AUDIO_STREAM_SYSTEM_AUDIO (2): System sounds, notifications
 * - Media playback (implied): Music, podcasts, other media
 *
 * Routing Strategy:
 * 1. PipeWire (primary): Handles stream priority, device routing, ducking
 * 2. PulseAudio (fallback): Legacy compatibility for older systems
 * 3. ALSA (fallback): Direct hardware access if no daemon available
 *
 * The router integrates with MediaPipeline to provide seamless audio
 * processing through the existing audio HAL.
 */
class AudioRouter : public QObject {
  Q_OBJECT

 public:
  explicit AudioRouter(MediaPipeline* mediaPipeline, QObject* parent = nullptr);
  ~AudioRouter() override;

  /**
   * @brief Initialize audio router with PipeWire/PulseAudio
   */
  bool initialize();

  /**
   * @brief Route audio data from AA channel to vehicle output
   *
   * @param role Audio stream role (media, guidance, system)
   * @param audioData PCM audio frames to route
   * @return true if audio successfully routed, false on error
   *
   * Audio data format from AASDK:
   * - Sample rate: 48 kHz (configurable, typically 16-48 kHz)
   * - Channels: Stereo (2) or Mono (1)
   * - Format: PCM 16-bit signed
   * - Endianness: Native (typically little-endian on ARM)
   */
  bool routeAudioFrame(AAudioStreamRole role, const QByteArray& audioData);

  /**
   * @brief Set target audio device for given stream role
   *
   * @param role Audio stream role
   * @param deviceId Device ID to route to (empty = default)
   * @return true if device set successfully
   */
  bool setAudioDevice(AAudioStreamRole role, const QString& deviceId);

  /**
   * @brief Get list of available audio output devices
   */
  QStringList getAvailableAudioDevices() const;

  /**
   * @brief Configure volume levels per audio stream
   *
   * @param role Audio stream role
   * @param volume Volume level (0-100)
   * @return true if volume set successfully
   */
  bool setStreamVolume(AAudioStreamRole role, int volume);

  /**
   * @brief Mute/unmute specific audio stream
   */
  bool setStreamMuted(AAudioStreamRole role, bool muted);

  /**
   * @brief Enable audio ducking for non-critical streams
   *
   * When guidance audio is active, automatically reduce volume
   * of media and system sounds to improve comprehension
   */
  bool enableAudioDucking(bool enable);

  /**
   * @brief Shutdown audio routing and cleanup resources
   */
  bool shutdown();

 signals:
  void audioDevicesChanged();
  void routingError(const QString& message);
  void streamStarted(AAudioStreamRole role);
  void streamStopped(AAudioStreamRole role);
  void volumeChanged(AAudioStreamRole role, int volume);

 private:
  /**
   * @brief Initialize PipeWire backend
   */
  bool initializePipeWire();

  /**
   * @brief Initialize PulseAudio backend
   */
  bool initializePulseAudio();

  /**
   * @brief Select appropriate audio device based on role
   */
  QAudioDevice selectAudioDevice(AAudioStreamRole role) const;

  /**
   * @brief Handle audio device hotplug events
   */
  void onAudioDevicesChanged();

  /**
   * @brief Handle MediaPipeline state changes
   */
  void onPipelineStateChanged(bool isActive);

  /**
   * @brief Convert AASDK audio stream type to router role
   */
  static AAudioStreamRole streamTypeToRole(int streamType);

  MediaPipeline* m_mediaPipeline = nullptr;
  bool m_initialized = false;

  // Audio device routing
  struct StreamConfig {
    QAudioDevice device;
    int volumeLevel = 80;
    bool muted = false;
    bool active = false;
  };

  StreamConfig m_mediaConfig;
  StreamConfig m_guidanceConfig;
  StreamConfig m_systemConfig;

  // Audio ducking state
  bool m_duckingEnabled = false;
  int m_duckingLevel = 40;  // Reduce non-critical streams to 40% during guidance

  // Backend detection
  bool m_pipewireAvailable = false;
  bool m_pulseaudioAvailable = false;
};
