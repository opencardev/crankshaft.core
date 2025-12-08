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

#include <QByteArray>
#include <QObject>
#include <memory>

/**
 * @brief Abstract interface for audio mixers
 * 
 * Provides a swappable interface for mixing multiple audio streams.
 * Supports volume control, muting, and automatic format conversion.
 */
class IAudioMixer : public QObject {
  Q_OBJECT

 public:
  enum class ChannelId {
    MEDIA = 0,      // Music playback
    SYSTEM = 1,     // System sounds, notifications
    SPEECH = 2,     // Navigation, voice assistant
    TELEPHONY = 3,  // Phone calls
    MAX_CHANNELS = 4
  };

  struct AudioFormat {
    int sampleRate{48000};
    int channels{2};  // 1=mono, 2=stereo
    int bitsPerSample{16};
  };

  struct ChannelConfig {
    ChannelId id;
    float volume{1.0f};     // 0.0 to 1.0
    bool muted{false};
    int priority{0};        // Higher = higher priority
    AudioFormat format;
  };

  explicit IAudioMixer(QObject* parent = nullptr) : QObject(parent) {}
  virtual ~IAudioMixer() = default;

  /**
   * @brief Initialize mixer with master audio format
   * @param masterFormat Output audio format
   * @return true if initialization successful
   */
  virtual bool initialize(const AudioFormat& masterFormat) = 0;

  /**
   * @brief Deinitialize and cleanup mixer resources
   */
  virtual void deinitialize() = 0;

  /**
   * @brief Add audio channel to mixer
   * @param config Channel configuration
   * @return true if channel added successfully
   */
  virtual bool addChannel(const ChannelConfig& config) = 0;

  /**
   * @brief Remove audio channel from mixer
   * @param channelId Channel to remove
   * @return true if channel removed successfully
   */
  virtual bool removeChannel(ChannelId channelId) = 0;

  /**
   * @brief Mix audio data from a specific channel
   * @param channelId Channel ID
   * @param audioData Audio data to mix
   * @return true if data mixed successfully
   */
  virtual bool mixAudioData(ChannelId channelId, const QByteArray& audioData) = 0;

  /**
   * @brief Set volume for a specific channel
   * @param channelId Channel ID
   * @param volume Volume (0.0 to 1.0)
   */
  virtual void setChannelVolume(ChannelId channelId, float volume) = 0;

  /**
   * @brief Get volume for a specific channel
   * @param channelId Channel ID
   * @return Current volume (0.0 to 1.0)
   */
  virtual float getChannelVolume(ChannelId channelId) const = 0;

  /**
   * @brief Mute or unmute a specific channel
   * @param channelId Channel ID
   * @param muted true to mute, false to unmute
   */
  virtual void setChannelMuted(ChannelId channelId, bool muted) = 0;

  /**
   * @brief Check if channel is muted
   * @param channelId Channel ID
   * @return true if channel is muted
   */
  virtual bool isChannelMuted(ChannelId channelId) const = 0;

  /**
   * @brief Set master volume (affects all channels)
   * @param volume Volume (0.0 to 1.0)
   */
  virtual void setMasterVolume(float volume) = 0;

  /**
   * @brief Get master volume
   * @return Current master volume (0.0 to 1.0)
   */
  virtual float getMasterVolume() const = 0;

  /**
   * @brief Check if mixer is initialized and ready
   * @return true if mixer is ready
   */
  virtual bool isReady() const = 0;

  /**
   * @brief Get mixer name for logging
   * @return Mixer implementation name
   */
  virtual QString getMixerName() const = 0;

 signals:
  /**
   * @brief Emitted when mixed audio data is available
   * @param mixedData Mixed audio data (master format)
   */
  void audioMixed(const QByteArray& mixedData);

  /**
   * @brief Emitted when mixer error occurs
   * @param error Error description
   */
  void errorOccurred(const QString& error);

  /**
   * @brief Emitted when channel configuration changes
   * @param channelId Channel that changed
   */
  void channelConfigChanged(ChannelId channelId);
};

using IAudioMixerPtr = std::shared_ptr<IAudioMixer>;

// Helper function to convert ChannelId to string
inline QString channelIdToString(IAudioMixer::ChannelId id) {
  switch (id) {
    case IAudioMixer::ChannelId::MEDIA:
      return "Media";
    case IAudioMixer::ChannelId::SYSTEM:
      return "System";
    case IAudioMixer::ChannelId::SPEECH:
      return "Speech";
    case IAudioMixer::ChannelId::TELEPHONY:
      return "Telephony";
    default:
      return "Unknown";
  }
}
