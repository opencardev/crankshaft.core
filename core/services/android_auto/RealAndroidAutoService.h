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

#include <QThread>
#include <boost/asio.hpp>
#include <memory>

#include "../../hal/multimedia/IAudioMixer.h"
#include "../../hal/multimedia/IVideoDecoder.h"
#include "AndroidAutoService.h"

// Forward declarations for AASDK
namespace aasdk {
namespace usb {
class IAOAPDevice;
class IUSBWrapper;
class IUSBHub;
class IAccessoryModeQueryFactory;
class IAccessoryModeQueryChainFactory;
}  // namespace usb
namespace messenger {
class IMessenger;
class ICryptor;
}  // namespace messenger
namespace transport {
class ITransport;
}  // namespace transport
namespace channel {
namespace mediasink {
namespace video {
class IVideoMediaSinkServiceEventHandler;
namespace channel {
class VideoChannel;
}  // namespace channel
}  // namespace video
namespace audio {
class IAudioMediaSinkServiceEventHandler;
namespace channel {
class MediaAudioChannel;
class SystemAudioChannel;
class GuidanceAudioChannel;
}  // namespace channel
}  // namespace audio
}  // namespace mediasink
namespace inputsource {
class IInputSourceServiceEventHandler;
class InputSourceService;
}  // namespace inputsource
namespace sensorsource {
class ISensorSourceServiceEventHandler;
class SensorSourceService;
}  // namespace sensorsource
namespace bluetooth {
class IBluetoothServiceEventHandler;
class BluetoothService;
}  // namespace bluetooth
namespace control {
class IControlServiceChannelEventHandler;
class ControlServiceChannel;
}  // namespace control
}  // namespace channel
}  // namespace aasdk

// Boost.Asio types included via headers

class MediaPipeline;

/**
 * @brief Real Android Auto service implementation using AASDK
 *
 * Implements full Android Auto protocol using AASDK library.
 * Handles USB device detection, AOAP protocol, and media streaming.
 */
class RealAndroidAutoService : public AndroidAutoService {
  Q_OBJECT

 public:
  explicit RealAndroidAutoService(MediaPipeline* mediaPipeline, QObject* parent = nullptr);
  ~RealAndroidAutoService() override;

  bool initialise() override;
  void deinitialise() override;

  ConnectionState getConnectionState() const override {
    return m_state;
  }
  bool isConnected() const override {
    return m_state == ConnectionState::CONNECTED;
  }
  AndroidDevice getConnectedDevice() const override {
    return m_device;
  }

  bool startSearching() override;
  void stopSearching() override;
  bool connectToDevice(const QString& serial) override;
  bool disconnect() override;

  bool setDisplayResolution(const QSize& resolution) override;
  QSize getDisplayResolution() const override {
    return m_resolution;
  }

  bool setFramerate(int fps) override;
  int getFramerate() const override {
    return m_fps;
  }

  bool sendTouchInput(int x, int y, int action) override;
  bool sendKeyInput(int key_code, int action) override;

  bool requestAudioFocus() override;
  bool abandonAudioFocus() override;

  int getFrameDropCount() const override {
    return m_droppedFrames;
  }
  int getLatency() const override {
    return m_latency;
  }

  bool setAudioEnabled(bool enabled) override;
  QJsonObject getAudioConfig() const override;

  // Channel configuration
  struct ChannelConfig {
    bool videoEnabled{true};
    bool mediaAudioEnabled{true};
    bool systemAudioEnabled{true};
    bool speechAudioEnabled{true};
    bool microphoneEnabled{true};
    bool inputEnabled{true};
    bool sensorEnabled{true};
    bool bluetoothEnabled{false};
  };

  void setChannelConfig(const ChannelConfig& config);
  ChannelConfig getChannelConfig() const {
    return m_channelConfig;
  }

 private:
  void setupAASDK();
  void cleanupAASDK();
  void setupChannels();
  void cleanupChannels();
  void handleDeviceDetected();
  void handleDeviceRemoved();
  void handleConnectionEstablished();
  void handleConnectionLost();
  void updateStats();
  void transitionToState(ConnectionState newState);

  // Channel event handlers
  void onVideoChannelUpdate(const QByteArray& data, int width, int height);
  void onMediaAudioChannelUpdate(const QByteArray& data);
  void onSystemAudioChannelUpdate(const QByteArray& data);
  void onSpeechAudioChannelUpdate(const QByteArray& data);
  void onSensorRequest();
  void onBluetoothPairingRequest(const QString& deviceName);
  void onChannelError(const QString& channelName, const QString& error);

  // AASDK callbacks
  void onVideoFrame(const uint8_t* data, int size, int width, int height);
  void onAudioData(const QByteArray& data);
  void onUSBHotplug(bool connected);

  ConnectionState m_state{ConnectionState::DISCONNECTED};
  AndroidDevice m_device;
  QSize m_resolution{1024, 600};
  int m_fps{30};
  bool m_audioEnabled{true};
  ChannelConfig m_channelConfig;

  // Statistics
  int m_droppedFrames{0};
  int m_latency{0};

  // AASDK components
  MediaPipeline* m_mediaPipeline{nullptr};
  std::shared_ptr<boost::asio::io_service> m_ioService;
  std::unique_ptr<QThread> m_aasdkThread;

  // Strands for channel operations
  std::unique_ptr<boost::asio::io_service::strand> m_strand;

  // Pointers to AASDK objects (owned by io_service)
  std::shared_ptr<aasdk::usb::IUSBWrapper> m_usbWrapper;
  std::shared_ptr<aasdk::usb::IAccessoryModeQueryFactory> m_queryFactory;
  std::shared_ptr<aasdk::usb::IAccessoryModeQueryChainFactory> m_queryChainFactory;
  std::shared_ptr<aasdk::usb::IUSBHub> m_usbHub;
  std::shared_ptr<aasdk::usb::IAOAPDevice> m_aoapDevice;
  std::shared_ptr<aasdk::transport::ITransport> m_transport;
  std::shared_ptr<aasdk::messenger::ICryptor> m_cryptor;
  std::shared_ptr<aasdk::messenger::IMessenger> m_messenger;

  // AASDK channels
  std::shared_ptr<aasdk::channel::mediasink::video::channel::VideoChannel> m_videoChannel;
  std::shared_ptr<aasdk::channel::mediasink::audio::channel::MediaAudioChannel> m_mediaAudioChannel;
  std::shared_ptr<aasdk::channel::mediasink::audio::channel::SystemAudioChannel>
      m_systemAudioChannel;
  std::shared_ptr<aasdk::channel::mediasink::audio::channel::GuidanceAudioChannel>
      m_speechAudioChannel;
  std::shared_ptr<aasdk::channel::inputsource::InputSourceService> m_inputChannel;
  std::shared_ptr<aasdk::channel::sensorsource::SensorSourceService> m_sensorChannel;
  std::shared_ptr<aasdk::channel::bluetooth::BluetoothService> m_bluetoothChannel;
  std::shared_ptr<aasdk::channel::control::ControlServiceChannel> m_controlChannel;

  // Multimedia components
  IVideoDecoder* m_videoDecoder{nullptr};
  IAudioMixer* m_audioMixer{nullptr};

  bool m_isInitialised{false};
};
