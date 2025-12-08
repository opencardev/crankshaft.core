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

#include "MediaPipeline.h"
#include <QDebug>

MediaPipeline::MediaPipeline(QObject* parent)
    : QObject(parent)
    , m_audioHAL(new AudioHAL(this))
    , m_videoHAL(new VideoHAL(this))
    , m_isActive(false) {
    
    // Connect audio HAL signals
    connect(m_audioHAL, &AudioHAL::volumeChanged, this, &MediaPipeline::onAudioVolumeChanged);
    connect(m_audioHAL, &AudioHAL::muteChanged, this, &MediaPipeline::onAudioMuteChanged);
    connect(m_audioHAL, &AudioHAL::routeChanged, this, &MediaPipeline::onAudioRouteChanged);
    connect(m_audioHAL, &AudioHAL::streamStarted, this, &MediaPipeline::onAudioStreamStarted);
    connect(m_audioHAL, &AudioHAL::streamStopped, this, &MediaPipeline::onAudioStreamStopped);
    connect(m_audioHAL, &AudioHAL::errorOccurred, this, &MediaPipeline::onAudioError);
    
    // Connect video HAL signals
    connect(m_videoHAL, &VideoHAL::resolutionChanged, this, &MediaPipeline::onVideoResolutionChanged);
    connect(m_videoHAL, &VideoHAL::brightnessChanged, this, &MediaPipeline::onVideoBrightnessChanged);
    connect(m_videoHAL, &VideoHAL::contrastChanged, this, &MediaPipeline::onVideoContrastChanged);
    connect(m_videoHAL, &VideoHAL::streamStarted, this, &MediaPipeline::onVideoStreamStarted);
    connect(m_videoHAL, &VideoHAL::streamStopped, this, &MediaPipeline::onVideoStreamStopped);
    connect(m_videoHAL, &VideoHAL::streamEnded, this, &MediaPipeline::onVideoStreamEnded);
    connect(m_videoHAL, &VideoHAL::errorOccurred, this, &MediaPipeline::onVideoError);
}

MediaPipeline::~MediaPipeline() {
    stop();
}

bool MediaPipeline::start(const MediaConfig& config) {
    if (m_isActive) {
        qWarning() << "Media pipeline already active";
        return false;
    }
    
    m_config = config;
    
    // Configure and start audio
    if (config.enableAudio) {
        m_audioHAL->setVolume(config.audioVolume);
        m_audioHAL->setRoute(config.audioRoute);
        
        if (!m_audioHAL->startStream(config.streamName, config.audioSampleRate, config.audioChannels)) {
            qCritical() << "Failed to start audio stream";
            return false;
        }
    }
    
    // Configure and start video
    if (config.enableVideo) {
        m_videoHAL->setResolution(config.videoResolution);
        m_videoHAL->setBrightness(config.videoBrightness);
        m_videoHAL->setContrast(config.videoContrast);
        
        if (!m_videoHAL->startVideoStream(config.streamName, config.videoCodec)) {
            qCritical() << "Failed to start video stream";
            if (config.enableAudio) {
                m_audioHAL->stopStream(config.streamName);
            }
            return false;
        }
    }
    
    m_isActive = true;
    qDebug() << "Media pipeline started successfully";
    emit pipelineStarted();
    return true;
}

bool MediaPipeline::stop() {
    if (!m_isActive) {
        return true;
    }
    
    // Stop audio
    if (m_config.enableAudio) {
        m_audioHAL->stopStream(m_config.streamName);
    }
    
    // Stop video
    if (m_config.enableVideo) {
        m_videoHAL->stopVideoStream(m_config.streamName);
    }
    
    m_isActive = false;
    qDebug() << "Media pipeline stopped";
    emit pipelineStopped();
    return true;
}

bool MediaPipeline::isActive() const {
    return m_isActive;
}

AudioHAL* MediaPipeline::audioHAL() const {
    return m_audioHAL;
}

VideoHAL* MediaPipeline::videoHAL() const {
    return m_videoHAL;
}

bool MediaPipeline::pushAudioData(const QByteArray& data) {
    if (!m_isActive || !m_config.enableAudio) {
        return false;
    }
    
    return m_audioHAL->pushAudioData(data);
}

bool MediaPipeline::pushVideoFrame(const QByteArray& frameData) {
    if (!m_isActive || !m_config.enableVideo) {
        return false;
    }
    
    return m_videoHAL->pushVideoFrame(frameData);
}

MediaConfig MediaPipeline::getConfig() const {
    return m_config;
}

bool MediaPipeline::updateConfig(const MediaConfig& config) {
    // Update audio settings without restarting
    if (config.enableAudio && m_config.enableAudio) {
        if (config.audioVolume != m_config.audioVolume) {
            m_audioHAL->setVolume(config.audioVolume);
        }
        if (config.audioRoute != m_config.audioRoute) {
            m_audioHAL->setRoute(config.audioRoute);
        }
    }
    
    // Update video settings without restarting
    if (config.enableVideo && m_config.enableVideo) {
        if (config.videoResolution != m_config.videoResolution) {
            m_videoHAL->setResolution(config.videoResolution);
        }
        if (config.videoBrightness != m_config.videoBrightness) {
            m_videoHAL->setBrightness(config.videoBrightness);
        }
        if (config.videoContrast != m_config.videoContrast) {
            m_videoHAL->setContrast(config.videoContrast);
        }
    }
    
    m_config = config;
    emit configUpdated();
    return true;
}

void MediaPipeline::onAudioVolumeChanged(int volume) {
    m_config.audioVolume = volume;
    emit audioVolumeChanged(volume);
}

void MediaPipeline::onAudioMuteChanged(bool muted) {
    emit audioMuteChanged(muted);
}

void MediaPipeline::onAudioRouteChanged(AudioHAL::AudioRoute route) {
    m_config.audioRoute = route;
    emit audioRouteChanged(route);
}

void MediaPipeline::onAudioStreamStarted(const QString& streamName) {
    qDebug() << "Audio stream started in pipeline:" << streamName;
}

void MediaPipeline::onAudioStreamStopped(const QString& streamName) {
    qDebug() << "Audio stream stopped in pipeline:" << streamName;
}

void MediaPipeline::onAudioError(const QString& error) {
    qCritical() << "Audio error in pipeline:" << error;
    emit errorOccurred("Audio: " + error);
}

void MediaPipeline::onVideoResolutionChanged(VideoHAL::VideoResolution resolution) {
    m_config.videoResolution = resolution;
    emit videoResolutionChanged(resolution);
}

void MediaPipeline::onVideoBrightnessChanged(int brightness) {
    m_config.videoBrightness = brightness;
    emit videoBrightnessChanged(brightness);
}

void MediaPipeline::onVideoContrastChanged(int contrast) {
    m_config.videoContrast = contrast;
    emit videoContrastChanged(contrast);
}

void MediaPipeline::onVideoStreamStarted(const QString& streamName) {
    qDebug() << "Video stream started in pipeline:" << streamName;
}

void MediaPipeline::onVideoStreamStopped(const QString& streamName) {
    qDebug() << "Video stream stopped in pipeline:" << streamName;
}

void MediaPipeline::onVideoStreamEnded() {
    qDebug() << "Video stream ended in pipeline";
    emit videoStreamEnded();
}

void MediaPipeline::onVideoError(const QString& error) {
    qCritical() << "Video error in pipeline:" << error;
    emit errorOccurred("Video: " + error);
}
