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

#include "VideoHAL.h"

#include <gst/app/gstappsrc.h>
#include <gst/gst.h>
#include <gst/video/video.h>

#include <QDebug>

class VideoHAL::VideoHALPrivate {
 public:
  GstElement* pipeline = nullptr;
  GstElement* source = nullptr;
  GstElement* decoder = nullptr;
  GstElement* convert = nullptr;
  GstElement* scale = nullptr;
  GstElement* sink = nullptr;
  GstBus* bus = nullptr;
  guint busWatchId = 0;

  VideoResolution currentResolution = VideoResolution::HD_720p;
  int currentBrightness = 50;
  int currentContrast = 50;
  bool isPlaying = false;
  QString currentVideoSink;

  static gboolean busCallback(GstBus* bus, GstMessage* message, gpointer userData);
  void updateBrightnessContrast();
};

gboolean VideoHAL::VideoHALPrivate::busCallback(GstBus* bus, GstMessage* message,
                                                gpointer userData) {
  VideoHAL* self = static_cast<VideoHAL*>(userData);

  switch (GST_MESSAGE_TYPE(message)) {
    case GST_MESSAGE_ERROR: {
      GError* err;
      gchar* debugInfo;
      gst_message_parse_error(message, &err, &debugInfo);
      qWarning() << "GStreamer video error from" << GST_OBJECT_NAME(message->src) << ":"
                 << err->message;
      qWarning() << "Debug info:" << (debugInfo ? debugInfo : "none");

      emit self->errorOccurred(QString("Video pipeline error: %1").arg(err->message));

      g_clear_error(&err);
      g_free(debugInfo);
      break;
    }
    case GST_MESSAGE_EOS:
      qDebug() << "Video stream reached end-of-stream";
      emit self->streamEnded();
      break;
    case GST_MESSAGE_STATE_CHANGED:
      if (GST_MESSAGE_SRC(message) == GST_OBJECT(self->d->pipeline)) {
        GstState oldState, newState, pending;
        gst_message_parse_state_changed(message, &oldState, &newState, &pending);
        qDebug() << "Video pipeline state changed from" << gst_element_state_get_name(oldState)
                 << "to" << gst_element_state_get_name(newState);

        if (newState == GST_STATE_PLAYING) {
          self->d->isPlaying = true;
        } else if (newState == GST_STATE_NULL || newState == GST_STATE_READY) {
          self->d->isPlaying = false;
        }
      }
      break;
    default:
      break;
  }

  return TRUE;
}

void VideoHAL::VideoHALPrivate::updateBrightnessContrast() {
  // Note: These properties may not be available on all sinks
  // videobalance element can be inserted for this purpose
  if (sink) {
    // Convert 0-100 range to -1.0 to 1.0 range
    double brightness = (currentBrightness - 50) / 50.0;
    double contrast = (currentContrast - 50) / 50.0 + 1.0;

    // Try to set properties if supported
    g_object_set(G_OBJECT(sink), "brightness", brightness, "contrast", contrast, nullptr);
  }
}

VideoHAL::VideoHAL(QObject* parent) : QObject(parent), d(new VideoHALPrivate()) {
  // Initialize GStreamer
  if (!gst_is_initialized()) {
    gst_init(nullptr, nullptr);
  }

  initializePipeline();
}

VideoHAL::~VideoHAL() {
  cleanup();
  delete d;
}

bool VideoHAL::initializePipeline() {
  // Create pipeline elements
  d->pipeline = gst_pipeline_new("video-pipeline");
  d->source = gst_element_factory_make("appsrc", "video-source");
  d->decoder = gst_element_factory_make("decodebin", "decoder");
  d->convert = gst_element_factory_make("videoconvert", "converter");
  d->scale = gst_element_factory_make("videoscale", "scaler");

  // Use waylandsink for Wayland or ximagesink for X11, fallback to autovideosink
  d->sink = gst_element_factory_make("waylandsink", "video-sink");
  if (!d->sink) {
    d->sink = gst_element_factory_make("autovideosink", "video-sink");
  }

  if (!d->pipeline || !d->source || !d->decoder || !d->convert || !d->scale || !d->sink) {
    qCritical() << "Failed to create GStreamer video elements";
    cleanup();
    return false;
  }

  // Build the pipeline (decoder will be linked dynamically via pad-added signal)
  gst_bin_add_many(GST_BIN(d->pipeline), d->source, d->decoder, d->convert, d->scale, d->sink,
                   nullptr);

  if (!gst_element_link(d->source, d->decoder)) {
    qCritical() << "Failed to link source to decoder";
    cleanup();
    return false;
  }

  if (!gst_element_link_many(d->convert, d->scale, d->sink, nullptr)) {
    qCritical() << "Failed to link convert, scale, and sink";
    cleanup();
    return false;
  }

  // Connect decoder pad-added signal for dynamic linking
  g_signal_connect(d->decoder, "pad-added",
                   G_CALLBACK(+[](GstElement* decoder, GstPad* pad, gpointer userData) {
                     VideoHALPrivate* priv = static_cast<VideoHALPrivate*>(userData);
                     GstPad* sinkPad = gst_element_get_static_pad(priv->convert, "sink");

                     if (!gst_pad_is_linked(sinkPad)) {
                       GstPadLinkReturn ret = gst_pad_link(pad, sinkPad);
                       if (ret != GST_PAD_LINK_OK) {
                         qWarning() << "Failed to link decoder pad to converter";
                       } else {
                         qDebug() << "Successfully linked decoder to converter";
                       }
                     }

                     gst_object_unref(sinkPad);
                   }),
                   d);

  // Set up bus monitoring
  d->bus = gst_pipeline_get_bus(GST_PIPELINE(d->pipeline));
  d->busWatchId = gst_bus_add_watch(d->bus, VideoHALPrivate::busCallback, this);
  gst_object_unref(d->bus);

  // Configure appsrc
  g_object_set(G_OBJECT(d->source), "stream-type", 0,  // GST_APP_STREAM_TYPE_STREAM
               "format", GST_FORMAT_TIME, "is-live", TRUE, nullptr);

  // Configure sink
  g_object_set(G_OBJECT(d->sink), "sync", TRUE, nullptr);

  qDebug() << "Video pipeline initialized successfully";
  return true;
}

void VideoHAL::cleanup() {
  if (d->busWatchId) {
    g_source_remove(d->busWatchId);
    d->busWatchId = 0;
  }

  if (d->pipeline) {
    gst_element_set_state(d->pipeline, GST_STATE_NULL);
    gst_object_unref(d->pipeline);
    d->pipeline = nullptr;
    d->source = nullptr;
    d->decoder = nullptr;
    d->convert = nullptr;
    d->scale = nullptr;
    d->sink = nullptr;
  }
}

bool VideoHAL::setResolution(VideoResolution resolution) {
  d->currentResolution = resolution;

  int width, height;
  switch (resolution) {
    case VideoResolution::SD_480p:
      width = 720;
      height = 480;
      break;
    case VideoResolution::HD_720p:
      width = 1280;
      height = 720;
      break;
    case VideoResolution::FullHD_1080p:
      width = 1920;
      height = 1080;
      break;
    case VideoResolution::UHD_4K:
      width = 3840;
      height = 2160;
      break;
    default:
      width = 1280;
      height = 720;
      break;
  }

  // Set caps on the scale element
  if (d->scale) {
    GstCaps* caps = gst_caps_new_simple("video/x-raw", "width", G_TYPE_INT, width, "height",
                                        G_TYPE_INT, height, nullptr);

    GstPad* srcPad = gst_element_get_static_pad(d->scale, "src");
    gst_pad_set_caps(srcPad, caps);
    gst_caps_unref(caps);
    gst_object_unref(srcPad);
  }

  qDebug() << "Video resolution set to" << width << "x" << height;
  emit resolutionChanged(resolution);
  return true;
}

VideoHAL::VideoResolution VideoHAL::getResolution() const {
  return d->currentResolution;
}

bool VideoHAL::setBrightness(int brightness) {
  if (brightness < 0 || brightness > 100) {
    qWarning() << "Invalid brightness level:" << brightness;
    return false;
  }

  d->currentBrightness = brightness;
  d->updateBrightnessContrast();

  qDebug() << "Video brightness set to" << brightness;
  emit brightnessChanged(brightness);
  return true;
}

int VideoHAL::getBrightness() const {
  return d->currentBrightness;
}

bool VideoHAL::setContrast(int contrast) {
  if (contrast < 0 || contrast > 100) {
    qWarning() << "Invalid contrast level:" << contrast;
    return false;
  }

  d->currentContrast = contrast;
  d->updateBrightnessContrast();

  qDebug() << "Video contrast set to" << contrast;
  emit contrastChanged(contrast);
  return true;
}

int VideoHAL::getContrast() const {
  return d->currentContrast;
}

bool VideoHAL::startVideoStream(const QString& streamName, const QString& codec) {
  if (!d->pipeline) {
    qWarning() << "Video pipeline not initialized";
    return false;
  }

  // Set caps based on codec
  GstCaps* caps = nullptr;
  if (codec == "h264") {
    caps = gst_caps_new_simple("video/x-h264", "stream-format", G_TYPE_STRING, "byte-stream",
                               "alignment", G_TYPE_STRING, "au", nullptr);
  } else if (codec == "h265") {
    caps = gst_caps_new_simple("video/x-h265", "stream-format", G_TYPE_STRING, "byte-stream",
                               "alignment", G_TYPE_STRING, "au", nullptr);
  } else if (codec == "vp8") {
    caps = gst_caps_new_simple("video/x-vp8", nullptr);
  } else if (codec == "vp9") {
    caps = gst_caps_new_simple("video/x-vp9", nullptr);
  } else {
    qWarning() << "Unsupported codec:" << codec;
    return false;
  }

  g_object_set(G_OBJECT(d->source), "caps", caps, nullptr);
  gst_caps_unref(caps);

  // Start pipeline
  GstStateChangeReturn ret = gst_element_set_state(d->pipeline, GST_STATE_PLAYING);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    qCritical() << "Failed to start video stream";
    return false;
  }

  qDebug() << "Video stream started:" << streamName << "with codec:" << codec;
  emit streamStarted(streamName);
  return true;
}

bool VideoHAL::stopVideoStream(const QString& streamName) {
  if (!d->pipeline) {
    return false;
  }

  GstStateChangeReturn ret = gst_element_set_state(d->pipeline, GST_STATE_NULL);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    qCritical() << "Failed to stop video stream";
    return false;
  }

  qDebug() << "Video stream stopped:" << streamName;
  emit streamStopped(streamName);
  return true;
}

bool VideoHAL::pushVideoFrame(const QByteArray& frameData) {
  if (!d->source || !d->isPlaying) {
    return false;
  }

  // Create GStreamer buffer
  GstBuffer* buffer = gst_buffer_new_allocate(nullptr, frameData.size(), nullptr);
  GstMapInfo map;
  gst_buffer_map(buffer, &map, GST_MAP_WRITE);
  memcpy(map.data, frameData.constData(), frameData.size());
  gst_buffer_unmap(buffer, &map);

  // Set timestamp
  static GstClockTime timestamp = 0;
  GST_BUFFER_PTS(buffer) = timestamp;
  GST_BUFFER_DURATION(buffer) = gst_util_uint64_scale_int(1, GST_SECOND, 30);  // Assume 30 fps
  timestamp += GST_BUFFER_DURATION(buffer);

  // Push buffer to appsrc
  GstFlowReturn ret = gst_app_src_push_buffer(GST_APP_SRC(d->source), buffer);

  if (ret != GST_FLOW_OK) {
    qWarning() << "Failed to push video frame, flow return:" << ret;
    return false;
  }

  return true;
}

QStringList VideoHAL::getSupportedCodecs() const {
  return QStringList() << "h264" << "h265" << "vp8" << "vp9";
}

bool VideoHAL::setVideoSink(const QString& sinkName) {
  if (d->currentVideoSink == sinkName) {
    return true;
  }

  // Recreate sink with new type
  if (d->pipeline && d->sink) {
    GstState state;
    gst_element_get_state(d->pipeline, &state, nullptr, GST_CLOCK_TIME_NONE);

    // Stop pipeline
    gst_element_set_state(d->pipeline, GST_STATE_NULL);

    // Remove old sink
    gst_element_unlink(d->scale, d->sink);
    gst_bin_remove(GST_BIN(d->pipeline), d->sink);

    // Create new sink
    d->sink = gst_element_factory_make(sinkName.toUtf8().constData(), "video-sink");
    if (!d->sink) {
      qCritical() << "Failed to create video sink:" << sinkName;
      d->sink = gst_element_factory_make("autovideosink", "video-sink");
      if (!d->sink) {
        return false;
      }
    }

    // Add and link new sink
    gst_bin_add(GST_BIN(d->pipeline), d->sink);
    if (!gst_element_link(d->scale, d->sink)) {
      qCritical() << "Failed to link video sink";
      return false;
    }

    // Configure sink
    g_object_set(G_OBJECT(d->sink), "sync", TRUE, nullptr);

    // Restore pipeline state
    gst_element_set_state(d->pipeline, state);

    d->currentVideoSink = sinkName;
    qDebug() << "Video sink changed to" << sinkName;
  }

  return true;
}
