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

#include "IAudioMixer.h"
#include <QMap>
#include <QMutex>
#include <QVector>

/**
 * @brief Software audio mixer implementation
 * 
 * Mixes multiple PCM audio streams with volume control and format conversion.
 * Supports mixing channels with different sample rates and channel counts.
 * Uses priority-based mixing when channels overlap.
 */
class AudioMixer : public IAudioMixer {
  Q_OBJECT

 public:
  explicit AudioMixer(QObject* parent = nullptr);
  ~AudioMixer() override;

  bool initialize(const AudioFormat& masterFormat) override;
  void deinitialize() override;

  bool addChannel(const ChannelConfig& config) override;
  bool removeChannel(ChannelId channelId) override;
  bool mixAudioData(ChannelId channelId, const QByteArray& audioData) override;

  void setChannelVolume(ChannelId channelId, float volume) override;
  float getChannelVolume(ChannelId channelId) const override;

  void setChannelMuted(ChannelId channelId, bool muted) override;
  bool isChannelMuted(ChannelId channelId) const override;

  void setMasterVolume(float volume) override;
  float getMasterVolume() const override { return m_masterVolume; }

  bool isReady() const override { return m_isInitialized; }
  QString getMixerName() const override { return "Software PCM Mixer"; }

 private:
  struct ChannelData {
    ChannelConfig config;
    QByteArray buffer;  // Buffered audio data
    bool active{false};
  };

  QByteArray convertFormat(const QByteArray& input,
                          const AudioFormat& inputFormat,
                          const AudioFormat& outputFormat);
  QByteArray resample(const QByteArray& input,
                     int inputSampleRate,
                     int outputSampleRate,
                     int channels,
                     int bitsPerSample);
  void mixBuffers();
  float applySaturation(float sample);

  AudioFormat m_masterFormat;
  float m_masterVolume{0.75f};
  bool m_isInitialized{false};

  QMap<ChannelId, ChannelData> m_channels;
  mutable QMutex m_mutex;

  // Mix buffer
  QByteArray m_mixBuffer;
  int m_mixBufferSamples{0};
};
