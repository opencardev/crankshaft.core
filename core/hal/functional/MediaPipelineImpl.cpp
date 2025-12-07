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

#include "../multimedia/MediaPipeline.h"
#include <QDebug>

// Placeholder implementation

class MediaPipelineImpl : public MediaPipeline {
 public:
  bool initialise(MediaType media_type) override {
    qDebug() << "[Media] Initialising pipeline for" << static_cast<int>(media_type);
    // TODO: Create GStreamer pipeline
    return true;
  }

  void deinitialise() override {
    qDebug() << "[Media] Deinitialising pipeline";
  }

  bool play() override { return true; }
  bool pause() override { return true; }
  bool stop() override { return true; }

  State getState() const override {
    return State::PLAYING;
  }

  bool linkAudioPath(AudioInputDevicePtr input,
                     AudioOutputDevicePtr output) override {
    return true;
  }

  bool linkVideoPath(VideoInputDevicePtr input,
                     VideoOutputDevicePtr output) override {
    return true;
  }

  void unlinkAudioPath() override {}
  void unlinkVideoPath() override {}

  double getCpuUsage() const override {
    return 25.5;
  }
};
