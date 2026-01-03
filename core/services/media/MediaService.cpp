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

#include "MediaService.h"

#include <QAudioOutput>
#include <QDir>
#include <QFile>
#include <QMediaMetaData>
#include <QUrl>

#include "../audio/AudioRouter.h"
#include "../logging/Logger.h"

MediaService::MediaService(AudioRouter* audioRouter, QObject* parent)
    : QObject(parent), m_audioRouter(audioRouter), m_logger(&Logger::instance()) {
  m_mediaPlayer = new QMediaPlayer(this);
  m_audioOutput = new QAudioOutput(this);
  m_mediaPlayer->setAudioOutput(m_audioOutput);

  // Connect QMediaPlayer signals
  connect(m_mediaPlayer, &QMediaPlayer::playbackStateChanged, this,
          &MediaService::onPlaybackStateChanged);
  connect(m_mediaPlayer, &QMediaPlayer::positionChanged, this, &MediaService::onPositionChanged);
  connect(m_mediaPlayer, &QMediaPlayer::durationChanged, this, &MediaService::onDurationChanged);
  connect(m_mediaPlayer, &QMediaPlayer::mediaStatusChanged, this,
          &MediaService::onMediaStatusChanged);
  connect(m_mediaPlayer, &QMediaPlayer::errorOccurred, this, &MediaService::onErrorOccurred);

  // Volume changed signal via QAudioOutput
  if (m_audioOutput) {
    connect(m_audioOutput, &QAudioOutput::volumeChanged, this, &MediaService::onVolumeChanged);
  }
}

MediaService::~MediaService() {
  shutdown();
}

bool MediaService::initialize() {
  if (!m_mediaPlayer || !m_audioOutput) {
    m_logger->error(QStringLiteral("[MediaService] Failed to initialise QMediaPlayer"));
    return false;
  }

  // Set initial volume (0.0-1.0 range in Qt6)
  m_audioOutput->setVolume(m_currentVolume / 100.0);
  m_logger->info(QStringLiteral("[MediaService] Initialised successfully"));
  return true;
}

bool MediaService::play(const QString& filePath) {
  if (!m_mediaPlayer) {
    m_logger->error(QStringLiteral("[MediaService] MediaPlayer not initialised"));
    return false;
  }

  QFile file(filePath);
  if (!file.exists()) {
    const QString error = QStringLiteral("[MediaService] File not found: ") + filePath;
    m_logger->warning(error);
    emit this->error(error);
    return false;
  }

  m_currentFilePath = filePath;
  m_currentTrackIndex = 0;

  // Store track list (all files in same directory)
  QDir dir(QFileInfo(filePath).absolutePath());
  const QStringList filters = {QStringLiteral("*.mp3"),  QStringLiteral("*.wav"),
                               QStringLiteral("*.flac"), QStringLiteral("*.m4a"),
                               QStringLiteral("*.ogg"),  QStringLiteral("*.aac")};
  m_trackList = dir.entryList(filters, QDir::Files | QDir::NoSymLinks);

  // Find current file in list
  m_currentTrackIndex = m_trackList.indexOf(QFileInfo(filePath).fileName());
  if (m_currentTrackIndex < 0) {
    m_currentTrackIndex = 0;
  }

  m_mediaPlayer->setSource(QUrl::fromLocalFile(filePath));
  m_mediaPlayer->play();

  updateMediaMetadata();
  m_logger->info(QStringLiteral("[MediaService] Playing: ") + filePath);
  return true;
}

void MediaService::pause() {
  if (!m_mediaPlayer) return;
  m_mediaPlayer->pause();
  m_logger->debug(QStringLiteral("[MediaService] Paused"));
}

void MediaService::resume() {
  if (!m_mediaPlayer) return;
  if (m_mediaPlayer->playbackState() == QMediaPlayer::PausedState) {
    m_mediaPlayer->play();
    m_logger->debug(QStringLiteral("[MediaService] Resumed"));
  }
}

void MediaService::skip(int direction) {
  if (m_trackList.isEmpty() || m_currentTrackIndex < 0) {
    return;
  }

  // Calculate next track index (with wrapping)
  int nextIndex = m_currentTrackIndex + direction;
  if (nextIndex < 0) {
    nextIndex = m_trackList.size() - 1;
  } else if (nextIndex >= m_trackList.size()) {
    nextIndex = 0;
  }

  if (nextIndex != m_currentTrackIndex && nextIndex >= 0 && nextIndex < m_trackList.size()) {
    m_currentTrackIndex = nextIndex;
    const QString nextFile = QFileInfo(m_currentFilePath).absolutePath() + QStringLiteral("/") +
                             m_trackList.at(nextIndex);

    play(nextFile);
    m_logger->debug(QStringLiteral("[MediaService] Skipped to: ") + m_trackList.at(nextIndex));
  }
}

void MediaService::seek(qint64 positionMs) {
  if (!m_mediaPlayer) return;
  m_mediaPlayer->setPosition(positionMs);
  m_logger->debug(QStringLiteral("[MediaService] Seeked to: %1ms").arg(positionMs));
}

void MediaService::setVolume(int level) {
  const int clampedLevel = qBound(0, level, 100);
  m_currentVolume = clampedLevel;

  if (m_audioOutput) {
    // Qt6 uses 0.0-1.0 range for volume
    m_audioOutput->setVolume(clampedLevel / 100.0);
    m_logger->debug(QStringLiteral("[MediaService] Volume set to: %1").arg(clampedLevel));
  }
}

int MediaService::volume() const {
  return m_currentVolume;
}

QVariantMap MediaService::currentMedia() const {
  QVariantMap info;
  info[QStringLiteral("filename")] = QFileInfo(m_currentFilePath).fileName();
  info[QStringLiteral("title")] =
      m_mediaTitle.isEmpty() ? info[QStringLiteral("filename")] : m_mediaTitle;
  info[QStringLiteral("artist")] = m_mediaArtist;
  info[QStringLiteral("album")] = m_mediaAlbum;
  info[QStringLiteral("duration")] = m_mediaDuration;
  info[QStringLiteral("position")] = m_currentPosition;
  info[QStringLiteral("state")] = m_mediaState;
  info[QStringLiteral("volume")] = m_currentVolume;
  return info;
}

QString MediaService::state() const {
  return m_mediaState;
}

QStringList MediaService::availableAudioDevices() const {
  if (m_audioRouter) {
    return m_audioRouter->getAvailableAudioDevices();
  }
  return QStringList();
}

bool MediaService::setAudioDevice(const QString& deviceId) {
  if (m_audioRouter) {
    m_audioRouter->setAudioDevice(AAudioStreamRole::MEDIA, deviceId);
    return true;
  }
  return false;
}

void MediaService::shutdown() {
  if (m_mediaPlayer) {
    m_mediaPlayer->stop();
    m_mediaPlayer->deleteLater();
    m_mediaPlayer = nullptr;
  }
  m_logger->info(QStringLiteral("[MediaService] Shutdown complete"));
}

void MediaService::onPlaybackStateChanged(QMediaPlayer::PlaybackState state) {
  QString newState;
  switch (state) {
    case QMediaPlayer::PlayingState:
      newState = QStringLiteral("playing");
      break;
    case QMediaPlayer::PausedState:
      newState = QStringLiteral("paused");
      break;
    case QMediaPlayer::StoppedState:
      newState = QStringLiteral("stopped");
      break;
    default:
      newState = QStringLiteral("unknown");
  }

  m_mediaState = newState;
  m_logger->debug(QStringLiteral("[MediaService] State changed to: ") + newState);
  emit stateChanged(newState);
  updateCurrentMediaInfo();
}

void MediaService::onPositionChanged(qint64 position) {
  m_currentPosition = position;
  emit positionChanged(position);
}

void MediaService::onDurationChanged(qint64 duration) {
  m_mediaDuration = duration;
  m_logger->debug(QStringLiteral("[MediaService] Duration: %1ms").arg(duration));
  emit durationChanged(duration);
  updateCurrentMediaInfo();
}

void MediaService::onMediaStatusChanged(QMediaPlayer::MediaStatus status) {
  switch (status) {
    case QMediaPlayer::LoadedMedia:
      updateMediaMetadata();
      break;
    case QMediaPlayer::InvalidMedia:
      m_logger->warning(QStringLiteral("[MediaService] Invalid media"));
      break;
    case QMediaPlayer::EndOfMedia:
      m_logger->debug(QStringLiteral("[MediaService] Media ended"));
      break;
    default:
      break;
  }
}

void MediaService::onVolumeChanged(float volume) {
  // Convert from 0.0-1.0 to 0-100
  m_currentVolume = static_cast<int>(volume * 100.0);
  emit volumeChanged(m_currentVolume);
}

void MediaService::onErrorOccurred(QMediaPlayer::Error error) {
  const QString errorMsg = m_mediaPlayer ? m_mediaPlayer->errorString() : QStringLiteral("Unknown");
  m_logger->error(QStringLiteral("[MediaService] Error: ") + errorMsg);
  m_mediaState = QStringLiteral("error");
  emit this->error(errorMsg);
}

void MediaService::updateMediaMetadata() {
  if (!m_mediaPlayer) {
    return;
  }

  // Qt6 returns QMediaMetaData object, access keys via value()
  const QMediaMetaData metadata = m_mediaPlayer->metaData();
  const QVariant titleVar = metadata.value(QMediaMetaData::Title);
  const QVariant artistVar = metadata.value(QMediaMetaData::ContributingArtist);
  const QVariant albumVar = metadata.value(QMediaMetaData::AlbumTitle);

  m_mediaTitle = titleVar.isValid() ? titleVar.toString() : QString();
  m_mediaArtist = artistVar.isValid() ? artistVar.toString() : QString();
  m_mediaAlbum = albumVar.isValid() ? albumVar.toString() : QString();

  // Fallback to filename if no title metadata
  if (m_mediaTitle.isEmpty()) {
    m_mediaTitle = QFileInfo(m_currentFilePath).baseName();
  }

  updateCurrentMediaInfo();
}

void MediaService::updateCurrentMediaInfo() {
  const QVariantMap info = currentMedia();
  emit mediaInfoChanged(info);
}
