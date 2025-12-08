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

#include <QObject>
#include <QString>
#include <QStringList>
#include <QByteArray>

/**
 * @brief Hardware Abstraction Layer for audio devices
 * 
 * Provides low-level audio hardware control and configuration.
 */
class AudioHAL : public QObject {
  Q_OBJECT

 public:
  enum class AudioRoute {
    Default,
    Speakers,
    Headphones,
    Bluetooth,
    USB
  };
  Q_ENUM(AudioRoute)

  explicit AudioHAL(QObject* parent = nullptr);
  ~AudioHAL() override;

  bool setVolume(int volume);
  int getVolume() const;
  
  bool setMute(bool muted);
  bool isMuted() const;
  
  bool setRoute(AudioRoute route);
  AudioRoute getCurrentRoute() const;
  
  bool startStream(const QString& streamName, int sampleRate, int channels);
  bool stopStream(const QString& streamName);
  bool pushAudioData(const QByteArray& data);
  
  QStringList getAvailableDevices() const;

 signals:
  void errorOccurred(const QString& message);
  void streamStarted(const QString& streamName);
  void streamStopped(const QString& streamName);
  void volumeChanged(int volume);
  void muteChanged(bool muted);
  void routeChanged(AudioRoute route);

 private:
  bool initializePipeline();
  void cleanup();

  class AudioHALPrivate;
  AudioHALPrivate* d;
};
