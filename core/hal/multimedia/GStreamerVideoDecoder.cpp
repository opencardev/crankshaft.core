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

#include "GStreamerVideoDecoder.h"

#include <QMutexLocker>

#include "../../services/logging/Logger.h"

GStreamerVideoDecoder::GStreamerVideoDecoder(QObject* parent) : IVideoDecoder(parent) {
  // Initialize GStreamer
  gst_init(nullptr, nullptr);
  Logger::instance().info("GStreamerVideoDecoder created");
}

GStreamerVideoDecoder::~GStreamerVideoDecoder() {
  deinitialize();
}

bool GStreamerVideoDecoder::initialize(const DecoderConfig& config) {
  if (m_isInitialized) {
    Logger::instance().warning("GStreamerVideoDecoder already initialized");
    return false;
  }

  m_config = config;

  if (!createPipeline()) {
    Logger::instance().error("Failed to create GStreamer pipeline");
    emit errorOccurred("Failed to create decoder pipeline");
    return false;
  }

  // Start pipeline
  GstStateChangeReturn ret = gst_element_set_state(m_pipeline, GST_STATE_PLAYING);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    Logger::instance().error("Failed to start GStreamer pipeline");
    destroyPipeline();
    emit errorOccurred("Failed to start decoder pipeline");
    return false;
  }

  m_isInitialized = true;
  Logger::instance().info(QString("GStreamerVideoDecoder initialized: %1x%2@%3fps, decoder=%4")
                              .arg(config.width)
                              .arg(config.height)
                              .arg(config.fps)
                              .arg(getDecoderElement()));

  return true;
}

void GStreamerVideoDecoder::deinitialize() {
  if (!m_isInitialized) {
    return;
  }

  destroyPipeline();
  m_isInitialized = false;
  Logger::instance().info("GStreamerVideoDecoder deinitialized");
}

bool GStreamerVideoDecoder::createPipeline() {
  // Create pipeline
  m_pipeline = gst_pipeline_new("video-decoder");
  if (!m_pipeline) {
    Logger::instance().error("Failed to create pipeline");
    return false;
  }

  // Create appsrc for feeding encoded data
  m_appSrc = gst_element_factory_make("appsrc", "source");
  if (!m_appSrc) {
    Logger::instance().error("Failed to create appsrc");
    return false;
  }

  // Configure appsrc
  g_object_set(G_OBJECT(m_appSrc), "stream-type", GST_APP_STREAM_TYPE_STREAM, "format",
               GST_FORMAT_TIME, "is-live", TRUE, "max-bytes",
               static_cast<guint64>(10 * 1024 * 1024),  // 10MB buffer
               nullptr);

  // Set H.264 caps on appsrc
  GstCaps* caps = gst_caps_new_simple("video/x-h264", "stream-format", G_TYPE_STRING, "byte-stream",
                                      "alignment", G_TYPE_STRING, "au", nullptr);
  g_object_set(G_OBJECT(m_appSrc), "caps", caps, nullptr);
  gst_caps_unref(caps);

  // Create h264parse
  m_h264Parse = gst_element_factory_make("h264parse", "parser");
  if (!m_h264Parse) {
    Logger::instance().error("Failed to create h264parse");
    return false;
  }

  // Create decoder (try hardware first, fall back to software)
  QString decoderName = getDecoderElement();
  m_decoder = gst_element_factory_make(decoderName.toStdString().c_str(), "decoder");
  if (!m_decoder) {
    Logger::instance().warning(
        QString("Hardware decoder %1 not available, falling back to avdec_h264").arg(decoderName));
    m_decoder = gst_element_factory_make("avdec_h264", "decoder");
  }

  if (!m_decoder) {
    Logger::instance().error("Failed to create decoder");
    return false;
  }

  // Create videoconvert for format conversion
  m_videoConvert = gst_element_factory_make("videoconvert", "convert");
  if (!m_videoConvert) {
    Logger::instance().error("Failed to create videoconvert");
    return false;
  }

  // Create appsink for receiving decoded frames
  m_appSink = gst_element_factory_make("appsink", "sink");
  if (!m_appSink) {
    Logger::instance().error("Failed to create appsink");
    return false;
  }

  // Configure appsink
  GstCaps* sinkCaps =
      gst_caps_new_simple("video/x-raw", "format", G_TYPE_STRING, "RGBA", "width", G_TYPE_INT,
                          m_config.width, "height", G_TYPE_INT, m_config.height, nullptr);
  g_object_set(G_OBJECT(m_appSink), "emit-signals", TRUE, "sync", FALSE, "max-buffers", 1, "drop",
               TRUE, "caps", sinkCaps, nullptr);
  gst_caps_unref(sinkCaps);

  // Connect new-sample signal
  g_signal_connect(m_appSink, "new-sample", G_CALLBACK(onNewSample), this);

  // Add elements to pipeline
  gst_bin_add_many(GST_BIN(m_pipeline), m_appSrc, m_h264Parse, m_decoder, m_videoConvert, m_appSink,
                   nullptr);

  // Link elements
  if (!gst_element_link(m_appSrc, m_h264Parse)) {
    Logger::instance().error("Failed to link appsrc to h264parse");
    return false;
  }

  if (!gst_element_link(m_h264Parse, m_decoder)) {
    Logger::instance().error("Failed to link h264parse to decoder");
    return false;
  }

  if (!gst_element_link(m_decoder, m_videoConvert)) {
    Logger::instance().error("Failed to link decoder to videoconvert");
    return false;
  }

  if (!gst_element_link(m_videoConvert, m_appSink)) {
    Logger::instance().error("Failed to link videoconvert to appsink");
    return false;
  }

  // Setup bus watch for error messages
  GstBus* bus = gst_pipeline_get_bus(GST_PIPELINE(m_pipeline));
  gst_bus_add_watch(bus, onBusMessage, this);
  gst_object_unref(bus);

  Logger::instance().info("GStreamer pipeline created successfully");
  return true;
}

void GStreamerVideoDecoder::destroyPipeline() {
  if (m_pipeline) {
    // Stop pipeline
    gst_element_set_state(m_pipeline, GST_STATE_NULL);

    // Unref pipeline (this will unref all child elements)
    gst_object_unref(GST_OBJECT(m_pipeline));

    m_pipeline = nullptr;
    m_appSrc = nullptr;
    m_h264Parse = nullptr;
    m_decoder = nullptr;
    m_videoConvert = nullptr;
    m_appSink = nullptr;
  }

  Logger::instance().info("GStreamer pipeline destroyed");
}

QString GStreamerVideoDecoder::getDecoderElement() const {
  if (!m_config.hardwareAcceleration) {
    return "avdec_h264";  // Software decoder
  }

  // Try to detect platform and use appropriate hardware decoder
  // Priority: VA-API > OMX > NVDEC > software

  // Check for VA-API support (Linux)
  GstElement* testElement = gst_element_factory_make("vaapih264dec", nullptr);
  if (testElement) {
    gst_object_unref(testElement);
    Logger::instance().info("Hardware decoder: vaapih264dec (VA-API)");
    return "vaapih264dec";
  }

  // Check for OMX support (Raspberry Pi)
  testElement = gst_element_factory_make("omxh264dec", nullptr);
  if (testElement) {
    gst_object_unref(testElement);
    Logger::instance().info("Hardware decoder: omxh264dec (OMX)");
    return "omxh264dec";
  }

  // Check for NVDEC support (NVIDIA)
  testElement = gst_element_factory_make("nvh264dec", nullptr);
  if (testElement) {
    gst_object_unref(testElement);
    Logger::instance().info("Hardware decoder: nvh264dec (NVDEC)");
    return "nvh264dec";
  }

  // Fall back to software
  Logger::instance().info("Hardware decoder: avdec_h264 (software fallback)");
  return "avdec_h264";
}

bool GStreamerVideoDecoder::decodeFrame(const QByteArray& encodedData) {
  if (!m_isInitialized || !m_appSrc) {
    Logger::instance().warning("Decoder not initialized");
    return false;
  }

  // Create GStreamer buffer from encoded data
  GstBuffer* buffer = gst_buffer_new_allocate(nullptr, encodedData.size(), nullptr);
  if (!buffer) {
    Logger::instance().error("Failed to allocate GStreamer buffer");
    m_droppedFrames++;
    return false;
  }

  // Copy data to buffer
  GstMapInfo map;
  if (!gst_buffer_map(buffer, &map, GST_MAP_WRITE)) {
    Logger::instance().error("Failed to map GStreamer buffer");
    gst_buffer_unref(buffer);
    m_droppedFrames++;
    return false;
  }

  memcpy(map.data, encodedData.data(), encodedData.size());
  gst_buffer_unmap(buffer, &map);

  // Push buffer to appsrc
  GstFlowReturn ret = gst_app_src_push_buffer(GST_APP_SRC(m_appSrc), buffer);
  if (ret != GST_FLOW_OK) {
    Logger::instance().error(
        QString("Failed to push buffer to appsrc: %1").arg(static_cast<int>(ret)));
    m_droppedFrames++;
    return false;
  }

  return true;
}

GstFlowReturn GStreamerVideoDecoder::onNewSample(GstAppSink* appsink, gpointer user_data) {
  GStreamerVideoDecoder* decoder = static_cast<GStreamerVideoDecoder*>(user_data);
  if (!decoder) {
    return GST_FLOW_ERROR;
  }

  // Pull sample from appsink
  GstSample* sample = gst_app_sink_pull_sample(appsink);
  if (!sample) {
    return GST_FLOW_ERROR;
  }

  // Get buffer from sample
  GstBuffer* buffer = gst_sample_get_buffer(sample);
  if (!buffer) {
    gst_sample_unref(sample);
    return GST_FLOW_ERROR;
  }

  // Get caps from sample
  GstCaps* caps = gst_sample_get_caps(sample);
  if (!caps) {
    gst_sample_unref(sample);
    return GST_FLOW_ERROR;
  }

  // Extract width and height
  GstStructure* structure = gst_caps_get_structure(caps, 0);
  int width = 0, height = 0;
  gst_structure_get_int(structure, "width", &width);
  gst_structure_get_int(structure, "height", &height);

  // Map buffer
  GstMapInfo map;
  if (!gst_buffer_map(buffer, &map, GST_MAP_READ)) {
    gst_sample_unref(sample);
    return GST_FLOW_ERROR;
  }

  // Emit decoded frame signal
  QMutexLocker locker(&decoder->m_mutex);
  decoder->m_decodedFrames++;

  // Emit signal with frame data
  emit decoder->frameDecoded(width, height, map.data, map.size);

  // Emit statistics every 30 frames
  if (decoder->m_decodedFrames % 30 == 0) {
    emit decoder->statsUpdated(decoder->m_decodedFrames, decoder->m_droppedFrames,
                               0.0);  // TODO: Calculate avg decode time
  }

  // Cleanup
  gst_buffer_unmap(buffer, &map);
  gst_sample_unref(sample);

  return GST_FLOW_OK;
}

gboolean GStreamerVideoDecoder::onBusMessage(GstBus* bus, GstMessage* message, gpointer user_data) {
  GStreamerVideoDecoder* decoder = static_cast<GStreamerVideoDecoder*>(user_data);
  if (!decoder) {
    return TRUE;
  }

  switch (GST_MESSAGE_TYPE(message)) {
    case GST_MESSAGE_ERROR: {
      GError* err = nullptr;
      gchar* debug = nullptr;
      gst_message_parse_error(message, &err, &debug);

      Logger::instance().error(QString("GStreamer error: %1").arg(err->message));
      if (debug) {
        Logger::instance().debug(QString("Debug info: %1").arg(debug));
      }

      emit decoder->errorOccurred(QString::fromUtf8(err->message));

      g_error_free(err);
      g_free(debug);
      break;
    }

    case GST_MESSAGE_WARNING: {
      GError* err = nullptr;
      gchar* debug = nullptr;
      gst_message_parse_warning(message, &err, &debug);

      Logger::instance().warning(QString("GStreamer warning: %1").arg(err->message));
      if (debug) {
        Logger::instance().debug(QString("Debug info: %1").arg(debug));
      }

      g_error_free(err);
      g_free(debug);
      break;
    }

    case GST_MESSAGE_EOS:
      Logger::instance().info("GStreamer: End of stream");
      break;

    case GST_MESSAGE_STATE_CHANGED: {
      if (GST_MESSAGE_SRC(message) == GST_OBJECT(decoder->m_pipeline)) {
        GstState oldState, newState, pending;
        gst_message_parse_state_changed(message, &oldState, &newState, &pending);
        Logger::instance().debug(QString("GStreamer state changed: %1 -> %2")
                                     .arg(gst_element_state_get_name(oldState))
                                     .arg(gst_element_state_get_name(newState)));
      }
      break;
    }

    default:
      break;
  }

  return TRUE;
}

void GStreamerVideoDecoder::onPadAdded(GstElement* element, GstPad* pad, gpointer data) {
  // This is for dynamic pad linking if needed
  Logger::instance().debug("Pad added to decoder");
}
