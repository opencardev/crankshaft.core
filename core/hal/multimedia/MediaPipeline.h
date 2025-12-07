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

#include "AudioDevice.h"
#include "VideoDevice.h"
#include <QString>
#include <QObject>
#include <memory>

/**
 * @brief Media pipeline for processing audio/video streams
 *
 * Abstracts GStreamer pipeline management for both audio and video.
 * Handles encoding, decoding, muxing, and demuxing of media streams.
 */
class MediaPipeline : public QObject {
  Q_OBJECT

 public:
  enum class State {
    NULL_STATE,
    READY,
    PAUSED,
    PLAYING,
    ERROR,
  };

  enum class MediaType {
    AUDIO_ONLY,
    VIDEO_ONLY,
    AUDIO_VIDEO,
  };

  explicit MediaPipeline(QObject* parent = nullptr);
  ~MediaPipeline() override;

  /**
   * @brief Initialise the pipeline
   * @param media_type Type of media to handle
   * @return true if initialisation successful
   */
  virtual bool initialise(MediaType media_type) = 0;

  /**
   * @brief Deinitialise the pipeline
   */
  virtual void deinitialise() = 0;

  /**
   * @brief Transition pipeline to PLAYING state
   */
  virtual bool play() = 0;

  /**
   * @brief Transition pipeline to PAUSED state
   */
  virtual bool pause() = 0;

  /**
   * @brief Transition pipeline to READY state
   */
  virtual bool stop() = 0;

  /**
   * @brief Get current pipeline state
   */
  virtual State getState() const = 0;

  /**
   * @brief Link audio input to output
   */
  virtual bool linkAudioPath(AudioInputDevicePtr input,
                             AudioOutputDevicePtr output) = 0;

  /**
   * @brief Link video input to output (with transcoding)
   */
  virtual bool linkVideoPath(VideoInputDevicePtr input,
                             VideoOutputDevicePtr output) = 0;

  /**
   * @brief Unlink audio path
   */
  virtual void unlinkAudioPath() = 0;

  /**
   * @brief Unlink video path
   */
  virtual void unlinkVideoPath() = 0;

  /**
   * @brief Get current CPU/memory usage
   */
  virtual double getCpuUsage() const = 0;

 signals:
  /**
   * @brief Emitted when pipeline state changes
   */
  void stateChanged(State state);

  /**
   * @brief Emitted when pipeline error occurs
   */
  void errorOccurred(const QString& error);

  /**
   * @brief Emitted when EOS (End Of Stream) is reached
   */
  void endOfStream();
};

using MediaPipelinePtr = std::shared_ptr<MediaPipeline>;
