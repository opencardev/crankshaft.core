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
#include <QString>
#include <memory>

#include "AudioHAL.h"
#include "VideoHAL.h"

/**
 * @brief Configuration for media pipeline
 */
struct MediaConfig {
  QString streamName;

  // Audio configuration
  bool enableAudio = false;
  int audioVolume = 100;
  AudioHAL::AudioRoute audioRoute = AudioHAL::AudioRoute::Default;
  int audioSampleRate = 48000;
  int audioChannels = 2;

  // Video configuration
  bool enableVideo = false;
  VideoHAL::VideoResolution videoResolution = VideoHAL::VideoResolution::HD_720p;
  int videoBrightness = 50;
  int videoContrast = 50;
  QString videoCodec = "H264";
};

/**
 * @brief Media pipeline for processing audio/video streams
 *
 * Coordinates audio and video HAL components for streaming media.
 * Manages configuration and data flow between components.
 */
class MediaPipeline : public QObject {
  Q_OBJECT

 public:
  explicit MediaPipeline(QObject* parent = nullptr);
  ~MediaPipeline() override;

  /**
   * @brief Start the media pipeline with given configuration
   */
  bool start(const MediaConfig& config);

  /**
   * @brief Stop the media pipeline
   */
  bool stop();

  /**
   * @brief Check if pipeline is active
   */
  bool isActive() const;

  /**
   * @brief Get audio HAL
   */
  AudioHAL* audioHAL() const;

  /**
   * @brief Get video HAL
   */
  VideoHAL* videoHAL() const;

  /**
   * @brief Push audio data to pipeline
   */
  bool pushAudioData(const QByteArray& data);

  /**
   * @brief Push video frame to pipeline
   */
  bool pushVideoFrame(const QByteArray& frameData);

  /**
   * @brief Get current configuration
   */
  MediaConfig getConfig() const;

  /**
   * @brief Update configuration dynamically
   */
  bool updateConfig(const MediaConfig& config);

 signals:
  void pipelineStarted();
  void pipelineStopped();
  void configUpdated();
  void audioVolumeChanged(int volume);
  void audioMuteChanged(bool muted);
  void audioRouteChanged(AudioHAL::AudioRoute route);
  void videoResolutionChanged(VideoHAL::VideoResolution resolution);
  void videoBrightnessChanged(int brightness);
  void videoContrastChanged(int contrast);
  void videoStreamEnded();
  void errorOccurred(const QString& error);

 private slots:
  void onAudioVolumeChanged(int volume);
  void onAudioMuteChanged(bool muted);
  void onAudioRouteChanged(AudioHAL::AudioRoute route);
  void onAudioStreamStarted(const QString& streamName);
  void onAudioStreamStopped(const QString& streamName);
  void onAudioError(const QString& error);

  void onVideoResolutionChanged(VideoHAL::VideoResolution resolution);
  void onVideoBrightnessChanged(int brightness);
  void onVideoContrastChanged(int contrast);
  void onVideoStreamStarted(const QString& streamName);
  void onVideoStreamStopped(const QString& streamName);
  void onVideoStreamEnded();
  void onVideoError(const QString& error);

 private:
  AudioHAL* m_audioHAL;
  VideoHAL* m_videoHAL;
  MediaConfig m_config;
  bool m_isActive;
};

using MediaPipelinePtr = std::shared_ptr<MediaPipeline>;
