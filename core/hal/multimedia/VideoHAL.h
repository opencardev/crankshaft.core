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
#include <QStringList>

/**
 * @brief Hardware Abstraction Layer for video devices
 *
 * Provides low-level video hardware control and configuration.
 */
class VideoHAL : public QObject {
  Q_OBJECT

 public:
  enum class VideoResolution { SD_480p, HD_720p, FullHD_1080p, UHD_4K };
  Q_ENUM(VideoResolution)

  explicit VideoHAL(QObject* parent = nullptr);
  ~VideoHAL() override;

  bool setResolution(VideoResolution resolution);
  VideoResolution getResolution() const;

  bool setBrightness(int brightness);
  int getBrightness() const;

  bool setContrast(int contrast);
  int getContrast() const;

  bool startVideoStream(const QString& streamName, const QString& codec);
  bool stopVideoStream(const QString& streamName);
  bool pushVideoFrame(const QByteArray& frameData);

  QStringList getSupportedCodecs() const;
  bool setVideoSink(const QString& sinkName);

 signals:
  void errorOccurred(const QString& message);
  void streamStarted(const QString& streamName);
  void streamStopped(const QString& streamName);
  void streamEnded();
  void resolutionChanged(VideoResolution resolution);
  void brightnessChanged(int brightness);
  void contrastChanged(int contrast);

 private:
  bool initializePipeline();
  void cleanup();

  class VideoHALPrivate;
  VideoHALPrivate* d;
};
