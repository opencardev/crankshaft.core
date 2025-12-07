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

#include "AudioHAL.h"
#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <QDebug>

class AudioHAL::AudioHALPrivate {
public:
    GstElement* pipeline = nullptr;
    GstElement* source = nullptr;
    GstElement* convert = nullptr;
    GstElement* volume = nullptr;
    GstElement* sink = nullptr;
    GstBus* bus = nullptr;
    guint busWatchId = 0;
    
    AudioHAL::AudioRoute currentRoute = AudioHAL::AudioRoute::Default;
    int currentVolume = 50;
    bool isMuted = false;
    
    static gboolean busCallback(GstBus* bus, GstMessage* message, gpointer userData);
};

gboolean AudioHAL::AudioHALPrivate::busCallback(GstBus* bus, GstMessage* message, gpointer userData) {
    AudioHAL* self = static_cast<AudioHAL*>(userData);
    
    switch (GST_MESSAGE_TYPE(message)) {
        case GST_MESSAGE_ERROR: {
            GError* err;
            gchar* debugInfo;
            gst_message_parse_error(message, &err, &debugInfo);
            qWarning() << "GStreamer error from" << GST_OBJECT_NAME(message->src) 
                       << ":" << err->message;
            qWarning() << "Debug info:" << (debugInfo ? debugInfo : "none");
            
            emit self->errorOccurred(QString("Audio pipeline error: %1").arg(err->message));
            
            g_clear_error(&err);
            g_free(debugInfo);
            break;
        }
        case GST_MESSAGE_EOS:
            qDebug() << "Audio stream reached end-of-stream";
            break;
        case GST_MESSAGE_STATE_CHANGED:
            if (GST_MESSAGE_SRC(message) == GST_OBJECT(self->d->pipeline)) {
                GstState oldState, newState, pending;
                gst_message_parse_state_changed(message, &oldState, &newState, &pending);
                qDebug() << "Audio pipeline state changed from" 
                         << gst_element_state_get_name(oldState) << "to"
                         << gst_element_state_get_name(newState);
            }
            break;
        default:
            break;
    }
    
    return TRUE;
}

AudioHAL::AudioHAL(QObject* parent)
    : QObject(parent)
    , d(new AudioHALPrivate()) {
    
    // Initialize GStreamer
    if (!gst_is_initialized()) {
        gst_init(nullptr, nullptr);
    }
    
    initializePipeline();
}

AudioHAL::~AudioHAL() {
    cleanup();
    delete d;
}

bool AudioHAL::initializePipeline() {
    // Create pipeline elements
    d->pipeline = gst_pipeline_new("audio-pipeline");
    d->source = gst_element_factory_make("appsrc", "audio-source");
    d->convert = gst_element_factory_make("audioconvert", "converter");
    d->volume = gst_element_factory_make("volume", "volume-control");
    d->sink = gst_element_factory_make("autoaudiosink", "audio-sink");
    
    if (!d->pipeline || !d->source || !d->convert || !d->volume || !d->sink) {
        qCritical() << "Failed to create GStreamer audio elements";
        cleanup();
        return false;
    }
    
    // Build the pipeline
    gst_bin_add_many(GST_BIN(d->pipeline), d->source, d->convert, d->volume, d->sink, nullptr);
    
    if (!gst_element_link_many(d->source, d->convert, d->volume, d->sink, nullptr)) {
        qCritical() << "Failed to link GStreamer audio elements";
        cleanup();
        return false;
    }
    
    // Set up bus monitoring
    d->bus = gst_pipeline_get_bus(GST_PIPELINE(d->pipeline));
    d->busWatchId = gst_bus_add_watch(d->bus, AudioHALPrivate::busCallback, this);
    gst_object_unref(d->bus);
    
    // Configure appsrc
    g_object_set(G_OBJECT(d->source),
                 "stream-type", 0, // GST_APP_STREAM_TYPE_STREAM
                 "format", GST_FORMAT_TIME,
                 "is-live", TRUE,
                 nullptr);
    
    // Set initial volume
    setVolume(d->currentVolume);
    
    qDebug() << "Audio pipeline initialized successfully";
    return true;
}

void AudioHAL::cleanup() {
    if (d->busWatchId) {
        g_source_remove(d->busWatchId);
        d->busWatchId = 0;
    }
    
    if (d->pipeline) {
        gst_element_set_state(d->pipeline, GST_STATE_NULL);
        gst_object_unref(d->pipeline);
        d->pipeline = nullptr;
        d->source = nullptr;
        d->convert = nullptr;
        d->volume = nullptr;
        d->sink = nullptr;
    }
}

bool AudioHAL::setVolume(int volume) {
    if (volume < 0 || volume > 100) {
        qWarning() << "Invalid volume level:" << volume;
        return false;
    }
    
    d->currentVolume = volume;
    
    if (d->volume) {
        double gstVolume = volume / 100.0;
        g_object_set(G_OBJECT(d->volume), "volume", gstVolume, nullptr);
        qDebug() << "Audio volume set to" << volume << "%";
        emit volumeChanged(volume);
        return true;
    }
    
    return false;
}

int AudioHAL::getVolume() const {
    return d->currentVolume;
}

bool AudioHAL::setMute(bool muted) {
    d->isMuted = muted;
    
    if (d->volume) {
        g_object_set(G_OBJECT(d->volume), "mute", muted, nullptr);
        qDebug() << "Audio mute set to" << muted;
        emit muteChanged(muted);
        return true;
    }
    
    return false;
}

bool AudioHAL::isMuted() const {
    return d->isMuted;
}

bool AudioHAL::setRoute(AudioRoute route) {
    d->currentRoute = route;
    
    // Route-specific configuration
    QString sinkName;
    switch (route) {
        case AudioRoute::Speakers:
            sinkName = "alsasink";
            break;
        case AudioRoute::Headphones:
            sinkName = "alsasink"; // Could be configured differently
            break;
        case AudioRoute::Bluetooth:
            sinkName = "pulsesink"; // Typically uses PulseAudio for Bluetooth
            break;
        case AudioRoute::USB:
            sinkName = "alsasink";
            break;
        case AudioRoute::Default:
        default:
            sinkName = "autoaudiosink";
            break;
    }
    
    // Recreate sink with new type
    if (d->pipeline && d->sink) {
        GstState state;
        gst_element_get_state(d->pipeline, &state, nullptr, GST_CLOCK_TIME_NONE);
        
        // Stop pipeline
        gst_element_set_state(d->pipeline, GST_STATE_NULL);
        
        // Remove old sink
        gst_bin_remove(GST_BIN(d->pipeline), d->sink);
        
        // Create new sink
        d->sink = gst_element_factory_make(sinkName.toUtf8().constData(), "audio-sink");
        if (!d->sink) {
            qCritical() << "Failed to create audio sink:" << sinkName;
            return false;
        }
        
        // Add and link new sink
        gst_bin_add(GST_BIN(d->pipeline), d->sink);
        if (!gst_element_link(d->volume, d->sink)) {
            qCritical() << "Failed to link audio sink";
            return false;
        }
        
        // Restore pipeline state
        gst_element_set_state(d->pipeline, state);
    }
    
    qDebug() << "Audio route changed to" << static_cast<int>(route);
    emit routeChanged(route);
    return true;
}

AudioHAL::AudioRoute AudioHAL::getCurrentRoute() const {
    return d->currentRoute;
}

bool AudioHAL::startStream(const QString& streamName, int sampleRate, int channels) {
    if (!d->pipeline) {
        qWarning() << "Audio pipeline not initialized";
        return false;
    }
    
    // Configure caps for the stream
    GstCaps* caps = gst_caps_new_simple("audio/x-raw",
                                        "format", G_TYPE_STRING, "S16LE",
                                        "rate", G_TYPE_INT, sampleRate,
                                        "channels", G_TYPE_INT, channels,
                                        nullptr);
    
    g_object_set(G_OBJECT(d->source), "caps", caps, nullptr);
    gst_caps_unref(caps);
    
    // Start pipeline
    GstStateChangeReturn ret = gst_element_set_state(d->pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        qCritical() << "Failed to start audio stream";
        return false;
    }
    
    qDebug() << "Audio stream started:" << streamName 
             << "(" << sampleRate << "Hz," << channels << "channels)";
    emit streamStarted(streamName);
    return true;
}

bool AudioHAL::stopStream(const QString& streamName) {
    if (!d->pipeline) {
        return false;
    }
    
    GstStateChangeReturn ret = gst_element_set_state(d->pipeline, GST_STATE_NULL);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        qCritical() << "Failed to stop audio stream";
        return false;
    }
    
    qDebug() << "Audio stream stopped:" << streamName;
    emit streamStopped(streamName);
    return true;
}

bool AudioHAL::pushAudioData(const QByteArray& data) {
    if (!d->source) {
        return false;
    }
    
    // Create GStreamer buffer
    GstBuffer* buffer = gst_buffer_new_allocate(nullptr, data.size(), nullptr);
    GstMapInfo map;
    gst_buffer_map(buffer, &map, GST_MAP_WRITE);
    memcpy(map.data, data.constData(), data.size());
    gst_buffer_unmap(buffer, &map);
    
    // Push buffer to appsrc
    GstFlowReturn ret = gst_app_src_push_buffer(GST_APP_SRC(d->source), buffer);
    
    if (ret != GST_FLOW_OK) {
        qWarning() << "Failed to push audio data, flow return:" << ret;
        return false;
    }
    
    return true;
}

QStringList AudioHAL::getAvailableDevices() const {
    QStringList devices;
    
    // Use GStreamer device monitor to list audio devices
    GstDeviceMonitor* monitor = gst_device_monitor_new();
    
    gst_device_monitor_add_filter(monitor, "Audio/Sink", nullptr);
    
    if (!gst_device_monitor_start(monitor)) {
        qWarning() << "Failed to start device monitor";
        gst_object_unref(monitor);
        return devices;
    }
    
    GList* deviceList = gst_device_monitor_get_devices(monitor);
    for (GList* item = deviceList; item != nullptr; item = item->next) {
        GstDevice* device = GST_DEVICE(item->data);
        gchar* name = gst_device_get_display_name(device);
        devices.append(QString::fromUtf8(name));
        g_free(name);
        gst_object_unref(device);
    }
    g_list_free(deviceList);
    
    gst_device_monitor_stop(monitor);
    gst_object_unref(monitor);
    
    return devices;
}
