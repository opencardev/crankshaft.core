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

#include <gst/app/gstappsink.h>
#include <gst/app/gstappsrc.h>
#include <gst/gst.h>

#include <QMutex>

#include "IVideoDecoder.h"

/**
 * @brief GStreamer-based video decoder
 *
 * Uses GStreamer pipeline for hardware-accelerated or software H.264 decoding.
 * Pipeline: appsrc ! h264parse ! avdec_h264 ! videoconvert ! video/x-raw,format=RGBA ! appsink
 *
 * Supports hardware acceleration via:
 * - VA-API (Linux)
 * - OMX (Raspberry Pi)
 * - NVDEC (NVIDIA)
 */
class GStreamerVideoDecoder : public IVideoDecoder {
  Q_OBJECT

 public:
  explicit GStreamerVideoDecoder(QObject* parent = nullptr);
  ~GStreamerVideoDecoder() override;

  bool initialize(const DecoderConfig& config) override;
  void deinitialize() override;
  bool decodeFrame(const QByteArray& encodedData) override;
  bool isReady() const override {
    return m_isInitialized;
  }
  QString getDecoderName() const override {
    return "GStreamer";
  }
  DecoderConfig getConfig() const override {
    return m_config;
  }

 private:
  bool createPipeline();
  void destroyPipeline();
  QString getDecoderElement() const;
  static GstFlowReturn onNewSample(GstAppSink* appsink, gpointer user_data);
  static void onPadAdded(GstElement* element, GstPad* pad, gpointer data);
  static gboolean onBusMessage(GstBus* bus, GstMessage* message, gpointer user_data);

  DecoderConfig m_config;
  bool m_isInitialized{false};

  // GStreamer pipeline elements
  GstElement* m_pipeline{nullptr};
  GstElement* m_appSrc{nullptr};
  GstElement* m_h264Parse{nullptr};
  GstElement* m_decoder{nullptr};
  GstElement* m_videoConvert{nullptr};
  GstElement* m_appSink{nullptr};

  // Statistics
  int m_decodedFrames{0};
  int m_droppedFrames{0};
  QMutex m_mutex;
};
