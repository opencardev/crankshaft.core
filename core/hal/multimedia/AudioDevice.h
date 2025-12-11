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

#include <QIODevice>
#include <QObject>
#include <QString>
#include <memory>

/**
 * @brief Base class for audio device abstraction
 *
 * Provides interface for audio input/output devices.
 * Implementations may use GStreamer, ALSA, PulseAudio, etc.
 */
class AudioDevice : public QObject {
  Q_OBJECT

 public:
  enum class AudioFormat {
    PCM_16,
    PCM_32,
    FLOAT,
  };

  enum class SampleRate {
    SR_8000 = 8000,
    SR_16000 = 16000,
    SR_44100 = 44100,
    SR_48000 = 48000,
  };

  enum class ChannelConfig {
    MONO = 1,
    STEREO = 2,
    SURROUND_5_1 = 6,
  };

  struct AudioConfig {
    AudioFormat format = AudioFormat::PCM_16;
    SampleRate sample_rate = SampleRate::SR_48000;
    ChannelConfig channels = ChannelConfig::STEREO;
    int buffer_size = 2048;
  };

  explicit AudioDevice(QObject* parent = nullptr) : QObject(parent) {}
  virtual ~AudioDevice() = default;

  /**
   * @brief Initialise the audio device
   * @param config Audio configuration
   * @return true if initialisation successful
   */
  virtual bool initialise(const AudioConfig& config) = 0;

  /**
   * @brief Deinitialise the audio device
   */
  virtual void deinitialise() = 0;

  /**
   * @brief Check if device is initialised
   */
  virtual bool isInitialised() const = 0;

  /**
   * @brief Get current audio configuration
   */
  virtual AudioConfig getConfig() const = 0;

  /**
   * @brief Get device name/identifier
   */
  virtual QString getDeviceName() const = 0;

 signals:
  /**
   * @brief Emitted when device error occurs
   */
  void errorOccurred(const QString& error);

  /**
   * @brief Emitted when device state changes
   */
  void stateChanged(const QString& state);
};

/**
 * @brief Output audio device (speaker, headphone)
 */
class AudioOutputDevice : public AudioDevice {
  Q_OBJECT

 public:
  explicit AudioOutputDevice(QObject* parent = nullptr) : AudioDevice(parent) {}
  ~AudioOutputDevice() override = default;

  /**
   * @brief Write audio data to output
   * @param data Audio buffer
   * @param size Size of buffer in bytes
   * @return Bytes written
   */
  virtual int write(const char* data, int size) = 0;

  /**
   * @brief Set output volume (0-100)
   */
  virtual void setVolume(int percent) = 0;

  /**
   * @brief Get current output volume
   */
  virtual int getVolume() const = 0;

  /**
   * @brief Mute/unmute output
   */
  virtual void setMuted(bool muted) = 0;

  /**
   * @brief Check if output is muted
   */
  virtual bool isMuted() const = 0;

 signals:
  /**
   * @brief Emitted when volume changes
   */
  void volumeChanged(int percent);

  /**
   * @brief Emitted when mute state changes
   */
  void muteStateChanged(bool muted);
};

/**
 * @brief Input audio device (microphone)
 */
class AudioInputDevice : public AudioDevice {
  Q_OBJECT

 public:
  explicit AudioInputDevice(QObject* parent = nullptr) : AudioDevice(parent) {}
  ~AudioInputDevice() override = default;

  /**
   * @brief Read audio data from input
   * @param buffer Destination buffer
   * @param size Maximum bytes to read
   * @return Bytes read
   */
  virtual int read(char* buffer, int size) = 0;

  /**
   * @brief Set input gain (0-100)
   */
  virtual void setGain(int percent) = 0;

  /**
   * @brief Get current input gain
   */
  virtual int getGain() const = 0;
};

using AudioOutputDevicePtr = std::shared_ptr<AudioOutputDevice>;
using AudioInputDevicePtr = std::shared_ptr<AudioInputDevice>;
