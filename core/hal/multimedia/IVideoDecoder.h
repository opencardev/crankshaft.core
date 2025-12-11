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
#include <memory>

/**
 * @brief Abstract interface for video decoders
 *
 * Provides a swappable interface for different video decoder implementations.
 * Implementations can use hardware acceleration (OMX, VAAPI, NVDEC) or
 * software decoding (FFmpeg, GStreamer).
 */
class IVideoDecoder : public QObject {
  Q_OBJECT

 public:
  enum class CodecType { H264, H265, VP8, VP9, AV1 };

  enum class PixelFormat { RGBA, RGB, NV12, YUV420P };

  struct DecoderConfig {
    CodecType codec{CodecType::H264};
    int width{1024};
    int height{600};
    int fps{30};
    PixelFormat outputFormat{PixelFormat::RGBA};
    bool hardwareAcceleration{true};
  };

  explicit IVideoDecoder(QObject* parent = nullptr) : QObject(parent) {}
  virtual ~IVideoDecoder() = default;

  /**
   * @brief Initialize decoder with configuration
   * @param config Decoder configuration
   * @return true if initialization successful
   */
  virtual bool initialize(const DecoderConfig& config) = 0;

  /**
   * @brief Deinitialize and cleanup decoder resources
   */
  virtual void deinitialize() = 0;

  /**
   * @brief Decode a video frame
   * @param encodedData Encoded frame data (H.264, H.265, etc.)
   * @return true if decoding successful
   */
  virtual bool decodeFrame(const QByteArray& encodedData) = 0;

  /**
   * @brief Check if decoder is initialized and ready
   * @return true if decoder is ready
   */
  virtual bool isReady() const = 0;

  /**
   * @brief Get decoder name for logging
   * @return Decoder implementation name
   */
  virtual QString getDecoderName() const = 0;

  /**
   * @brief Get current decoder configuration
   * @return Current configuration
   */
  virtual DecoderConfig getConfig() const = 0;

 signals:
  /**
   * @brief Emitted when a frame is successfully decoded
   * @param width Frame width
   * @param height Frame height
   * @param data Decoded frame data (format specified in config)
   * @param size Data size in bytes
   */
  void frameDecoded(int width, int height, const uint8_t* data, int size);

  /**
   * @brief Emitted when decoder error occurs
   * @param error Error description
   */
  void errorOccurred(const QString& error);

  /**
   * @brief Emitted when decoder statistics are updated
   * @param decodedFrames Total frames decoded
   * @param droppedFrames Total frames dropped
   * @param avgDecodeTime Average decode time in ms
   */
  void statsUpdated(int decodedFrames, int droppedFrames, double avgDecodeTime);
};

using IVideoDecoderPtr = std::shared_ptr<IVideoDecoder>;
