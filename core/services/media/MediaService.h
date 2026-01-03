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

#include <QMediaPlayer>
#include <QObject>
#include <QString>
#include <QVariantMap>

class Logger;
class AudioRouter;

/**
 * MediaService — Native media player service for Crankshaft infotainment.
 *
 * Provides playback control for audio and video files with support for:
 * - Play, pause, resume, skip (next/previous), seek operations
 * - Volume control (0-100)
 * - Now-playing metadata (title, artist, duration)
 * - Audio routing integration via AudioRouter
 * - WebSocket command interface for remote control
 *
 * Performance target: <200ms latency from UI control to state change
 */
class MediaService : public QObject {
  Q_OBJECT

 public:
  explicit MediaService(AudioRouter* audioRouter = nullptr, QObject* parent = nullptr);
  ~MediaService() override;

  /**
   * Initialize MediaService and detect available audio devices.
   * @return true if successfully initialised, false if critical error
   */
  bool initialize();

  /**
   * Load and start playback of media file.
   * @param filePath Absolute path to media file (mp3, wav, flac, m4a, etc.)
   * @return true if playback started, false if file not found or unsupported
   */
  bool play(const QString& filePath);

  /**
   * Pause current playback without stopping.
   * Resume with resume() to continue from same position.
   */
  void pause();

  /**
   * Resume playback from paused state.
   */
  void resume();

  /**
   * Skip to next or previous track in current directory.
   * @param direction 1 for next, -1 for previous
   */
  void skip(int direction);

  /**
   * Seek to specific position in current media.
   * @param positionMs Desired position in milliseconds
   */
  void seek(qint64 positionMs);

  /**
   * Set playback volume level (0-100).
   * @param level Volume level (0 = mute, 100 = max)
   */
  void setVolume(int level);

  /**
   * Get current playback volume level.
   * @return Current volume (0-100)
   */
  int volume() const;

  /**
   * Get current media information.
   * @return QVariantMap with keys: filename, title, artist, duration, position, state
   */
  [[nodiscard]] QVariantMap currentMedia() const;

  /**
   * Get playback state.
   * @return "playing", "paused", "stopped", "error"
   */
  [[nodiscard]] QString state() const;

  /**
   * Get list of available audio output devices.
   * @return QStringList of device names
   */
  [[nodiscard]] QStringList availableAudioDevices() const;

  /**
   * Set audio output device for media playback.
   * @param deviceId Device identifier (from availableAudioDevices)
   * @return true if device set successfully
   */
  bool setAudioDevice(const QString& deviceId);

  /**
   * Shutdown MediaService and release resources.
   */
  void shutdown();

 signals:
  /**
   * Emitted when playback state changes.
   */
  void stateChanged(const QString& newState);

  /**
   * Emitted when current position changes during playback.
   */
  void positionChanged(qint64 positionMs);

  /**
   * Emitted when media duration is determined.
   */
  void durationChanged(qint64 durationMs);

  /**
   * Emitted when now-playing metadata is updated.
   */
  void mediaInfoChanged(const QVariantMap& info);

  /**
   * Emitted when volume changes.
   */
  void volumeChanged(int level);

  /**
   * Emitted when available audio devices change.
   */
  void audioDevicesChanged();

  /**
   * Emitted on playback error.
   */
  void error(const QString& message);

 private slots:
  void onPlaybackStateChanged(QMediaPlayer::PlaybackState state);
  void onPositionChanged(qint64 position);
  void onDurationChanged(qint64 duration);
  void onMediaStatusChanged(QMediaPlayer::MediaStatus status);
  void onVolumeChanged(float volume);
  void onErrorOccurred(QMediaPlayer::Error error);

 private:
  /**
   * Extract metadata (title, artist) from media file.
   * Uses Qt Media service if available, falls back to filename.
   */
  void updateMediaMetadata();

  /**
   * Update current media information QVariantMap.
   */
  void updateCurrentMediaInfo();

  QMediaPlayer* m_mediaPlayer{nullptr};
  QAudioOutput* m_audioOutput{nullptr};
  AudioRouter* m_audioRouter{nullptr};
  Logger* m_logger{nullptr};

  // Current media state
  QString m_currentFilePath;
  QString m_mediaState{QStringLiteral("stopped")};
  int m_currentVolume{80};
  qint64 m_currentPosition{0};
  qint64 m_mediaDuration{0};

  // Metadata
  QString m_mediaTitle;
  QString m_mediaArtist;
  QString m_mediaAlbum;

  // Track list for skip functionality
  QStringList m_trackList;
  int m_currentTrackIndex{-1};
};
