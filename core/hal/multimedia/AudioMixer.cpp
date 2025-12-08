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

#include "AudioMixer.h"
#include "../../services/logging/Logger.h"
#include <QMutexLocker>
#include <cmath>
#include <cstring>

AudioMixer::AudioMixer(QObject* parent)
    : IAudioMixer(parent) {
  Logger::instance().info("AudioMixer created");
}

AudioMixer::~AudioMixer() {
  deinitialize();
}

bool AudioMixer::initialize(const AudioFormat& masterFormat) {
  if (m_isInitialized) {
    Logger::instance().warning("AudioMixer already initialized");
    return false;
  }

  m_masterFormat = masterFormat;
  m_isInitialized = true;

  Logger::instance().info(
      QString("AudioMixer initialized: %1Hz, %2ch, %3bit")
          .arg(masterFormat.sampleRate)
          .arg(masterFormat.channels)
          .arg(masterFormat.bitsPerSample));

  return true;
}

void AudioMixer::deinitialize() {
  if (!m_isInitialized) {
    return;
  }

  QMutexLocker locker(&m_mutex);
  m_channels.clear();
  m_mixBuffer.clear();
  m_isInitialized = false;

  Logger::instance().info("AudioMixer deinitialized");
}

bool AudioMixer::addChannel(const ChannelConfig& config) {
  QMutexLocker locker(&m_mutex);

  if (m_channels.contains(config.id)) {
    Logger::instance().warning(
        QString("Channel %1 already exists").arg(channelIdToString(config.id)));
    return false;
  }

  ChannelData data;
  data.config = config;
  data.active = false;

  m_channels.insert(config.id, data);

  Logger::instance().info(
      QString("Added audio channel: %1 (%2Hz, %3ch, %4bit, volume=%5, priority=%6)")
          .arg(channelIdToString(config.id))
          .arg(config.format.sampleRate)
          .arg(config.format.channels)
          .arg(config.format.bitsPerSample)
          .arg(config.volume)
          .arg(config.priority));

  emit channelConfigChanged(config.id);
  return true;
}

bool AudioMixer::removeChannel(ChannelId channelId) {
  QMutexLocker locker(&m_mutex);

  if (!m_channels.contains(channelId)) {
    Logger::instance().warning(
        QString("Channel %1 does not exist").arg(channelIdToString(channelId)));
    return false;
  }

  m_channels.remove(channelId);
  Logger::instance().info(
      QString("Removed audio channel: %1").arg(channelIdToString(channelId)));

  return true;
}

bool AudioMixer::mixAudioData(ChannelId channelId, const QByteArray& audioData) {
  if (!m_isInitialized) {
    return false;
  }

  QMutexLocker locker(&m_mutex);

  if (!m_channels.contains(channelId)) {
    Logger::instance().warning(
        QString("Cannot mix audio: channel %1 not found").arg(channelIdToString(channelId)));
    return false;
  }

  ChannelData& channel = m_channels[channelId];

  // Convert format if needed
  QByteArray convertedData = audioData;
  if (channel.config.format.sampleRate != m_masterFormat.sampleRate ||
      channel.config.format.channels != m_masterFormat.channels ||
      channel.config.format.bitsPerSample != m_masterFormat.bitsPerSample) {
    convertedData = convertFormat(audioData, channel.config.format, m_masterFormat);
  }

  // Append to channel buffer
  channel.buffer.append(convertedData);
  channel.active = true;

  // Mix all active channels
  mixBuffers();

  return true;
}

void AudioMixer::setChannelVolume(ChannelId channelId, float volume) {
  QMutexLocker locker(&m_mutex);

  if (!m_channels.contains(channelId)) {
    return;
  }

  volume = qBound(0.0f, volume, 1.0f);
  m_channels[channelId].config.volume = volume;

  Logger::instance().debug(
      QString("Channel %1 volume set to %2").arg(channelIdToString(channelId)).arg(volume));

  emit channelConfigChanged(channelId);
}

float AudioMixer::getChannelVolume(ChannelId channelId) const {
  QMutexLocker locker(&m_mutex);

  if (!m_channels.contains(channelId)) {
    return 0.0f;
  }

  return m_channels[channelId].config.volume;
}

void AudioMixer::setChannelMuted(ChannelId channelId, bool muted) {
  QMutexLocker locker(&m_mutex);

  if (!m_channels.contains(channelId)) {
    return;
  }

  m_channels[channelId].config.muted = muted;

  Logger::instance().debug(
      QString("Channel %1 %2").arg(channelIdToString(channelId)).arg(muted ? "muted" : "unmuted"));

  emit channelConfigChanged(channelId);
}

bool AudioMixer::isChannelMuted(ChannelId channelId) const {
  QMutexLocker locker(&m_mutex);

  if (!m_channels.contains(channelId)) {
    return true;
  }

  return m_channels[channelId].config.muted;
}

void AudioMixer::setMasterVolume(float volume) {
  volume = qBound(0.0f, volume, 1.0f);
  m_masterVolume = volume;

  Logger::instance().debug(QString("Master volume set to %1").arg(volume));
}

void AudioMixer::mixBuffers() {
  // Calculate minimum buffer size across all active channels
  int minBufferSize = INT_MAX;
  int activeChannelCount = 0;

  for (auto it = m_channels.begin(); it != m_channels.end(); ++it) {
    if (it->active && it->buffer.size() > 0) {
      minBufferSize = qMin(minBufferSize, it->buffer.size());
      activeChannelCount++;
    }
  }

  if (activeChannelCount == 0 || minBufferSize == 0) {
    return;
  }

  // Calculate number of samples to mix
  int bytesPerSample = m_masterFormat.bitsPerSample / 8;
  int sampleCount = minBufferSize / (bytesPerSample * m_masterFormat.channels);

  if (sampleCount == 0) {
    return;
  }

  // Prepare mix buffer
  int mixBufferSize = sampleCount * bytesPerSample * m_masterFormat.channels;
  m_mixBuffer.resize(mixBufferSize);
  m_mixBuffer.fill(0);

  // Mix samples (16-bit PCM assumed)
  if (m_masterFormat.bitsPerSample == 16) {
    int16_t* mixBufferPtr = reinterpret_cast<int16_t*>(m_mixBuffer.data());

    // Sort channels by priority (highest first)
    QList<ChannelId> sortedChannels;
    for (auto it = m_channels.begin(); it != m_channels.end(); ++it) {
      if (it->active && it->buffer.size() >= mixBufferSize) {
        sortedChannels.append(it.key());
      }
    }

    std::sort(sortedChannels.begin(), sortedChannels.end(),
              [this](ChannelId a, ChannelId b) {
                return m_channels[a].config.priority > m_channels[b].config.priority;
              });

    // Mix each channel
    for (ChannelId channelId : sortedChannels) {
      ChannelData& channel = m_channels[channelId];

      if (channel.config.muted) {
        continue;
      }

      const int16_t* channelPtr = reinterpret_cast<const int16_t*>(channel.buffer.constData());
      float channelVolume = channel.config.volume * m_masterVolume;

      // Mix samples
      for (int i = 0; i < sampleCount * m_masterFormat.channels; ++i) {
        float sample = static_cast<float>(mixBufferPtr[i]) + 
                      (static_cast<float>(channelPtr[i]) * channelVolume);
        
        // Apply soft saturation
        sample = applySaturation(sample);
        
        mixBufferPtr[i] = static_cast<int16_t>(sample);
      }
    }
  }

  // Remove mixed data from channel buffers
  for (auto it = m_channels.begin(); it != m_channels.end(); ++it) {
    if (it->active && it->buffer.size() >= mixBufferSize) {
      it->buffer.remove(0, mixBufferSize);
      
      if (it->buffer.isEmpty()) {
        it->active = false;
      }
    }
  }

  // Emit mixed audio
  if (!m_mixBuffer.isEmpty()) {
    emit audioMixed(m_mixBuffer);
  }
}

float AudioMixer::applySaturation(float sample) {
  // Soft clipping to avoid harsh distortion
  const float maxValue = 32767.0f;
  const float minValue = -32768.0f;

  if (sample > maxValue) {
    // Soft clip positive samples
    float excess = sample - maxValue;
    sample = maxValue - (excess * 0.5f);
    if (sample > maxValue) {
      sample = maxValue;
    }
  } else if (sample < minValue) {
    // Soft clip negative samples
    float excess = minValue - sample;
    sample = minValue + (excess * 0.5f);
    if (sample < minValue) {
      sample = minValue;
    }
  }

  return sample;
}

QByteArray AudioMixer::convertFormat(const QByteArray& input,
                                    const AudioFormat& inputFormat,
                                    const AudioFormat& outputFormat) {
  // Simple format conversion (resampling + channel conversion)
  QByteArray result = input;

  // First, resample if sample rates differ
  if (inputFormat.sampleRate != outputFormat.sampleRate) {
    result = resample(result, inputFormat.sampleRate, outputFormat.sampleRate,
                     inputFormat.channels, inputFormat.bitsPerSample);
  }

  // Then, convert channels if needed (mono<->stereo)
  if (inputFormat.channels != outputFormat.channels) {
    if (inputFormat.channels == 1 && outputFormat.channels == 2) {
      // Mono to stereo: duplicate samples
      QByteArray stereo;
      stereo.reserve(result.size() * 2);
      
      if (inputFormat.bitsPerSample == 16) {
        const int16_t* monoPtr = reinterpret_cast<const int16_t*>(result.constData());
        int sampleCount = result.size() / 2;
        
        for (int i = 0; i < sampleCount; ++i) {
          int16_t sample = monoPtr[i];
          stereo.append(reinterpret_cast<const char*>(&sample), 2);
          stereo.append(reinterpret_cast<const char*>(&sample), 2);
        }
      }
      
      result = stereo;
    } else if (inputFormat.channels == 2 && outputFormat.channels == 1) {
      // Stereo to mono: average samples
      QByteArray mono;
      mono.reserve(result.size() / 2);
      
      if (inputFormat.bitsPerSample == 16) {
        const int16_t* stereoPtr = reinterpret_cast<const int16_t*>(result.constData());
        int sampleCount = result.size() / 4;  // 2 samples per frame, 2 bytes each
        
        for (int i = 0; i < sampleCount; ++i) {
          int16_t left = stereoPtr[i * 2];
          int16_t right = stereoPtr[i * 2 + 1];
          int16_t avg = (left + right) / 2;
          mono.append(reinterpret_cast<const char*>(&avg), 2);
        }
      }
      
      result = mono;
    }
  }

  return result;
}

QByteArray AudioMixer::resample(const QByteArray& input,
                               int inputSampleRate,
                               int outputSampleRate,
                               int channels,
                               int bitsPerSample) {
  // Simple linear interpolation resampling
  if (inputSampleRate == outputSampleRate) {
    return input;
  }

  int bytesPerSample = bitsPerSample / 8;
  int inputSampleCount = input.size() / (bytesPerSample * channels);
  int outputSampleCount = (inputSampleCount * outputSampleRate) / inputSampleRate;

  QByteArray output;
  output.resize(outputSampleCount * bytesPerSample * channels);

  if (bitsPerSample == 16) {
    const int16_t* inputPtr = reinterpret_cast<const int16_t*>(input.constData());
    int16_t* outputPtr = reinterpret_cast<int16_t*>(output.data());

    float ratio = static_cast<float>(inputSampleRate) / outputSampleRate;

    for (int i = 0; i < outputSampleCount; ++i) {
      float srcIndex = i * ratio;
      int srcIndexInt = static_cast<int>(srcIndex);
      float frac = srcIndex - srcIndexInt;

      for (int ch = 0; ch < channels; ++ch) {
        if (srcIndexInt + 1 < inputSampleCount) {
          float sample1 = inputPtr[srcIndexInt * channels + ch];
          float sample2 = inputPtr[(srcIndexInt + 1) * channels + ch];
          float interpolated = sample1 + (sample2 - sample1) * frac;
          outputPtr[i * channels + ch] = static_cast<int16_t>(interpolated);
        } else {
          outputPtr[i * channels + ch] = inputPtr[srcIndexInt * channels + ch];
        }
      }
    }
  }

  return output;
}
