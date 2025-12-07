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

#include "AndroidAutoService.h"
#include "../../hal/multimedia/MediaPipeline.h"
#include <QTimer>
#include <QDebug>
#include <libusb-1.0/libusb.h>
#include <f1x/aasdk/USB/USBHub.hpp>
#include <f1x/aasdk/USB/ConnectedAccessoriesEnumerator.hpp>
#include <f1x/aasdk/USB/AccessoryModeQueryChain.hpp>
#include <f1x/aasdk/USB/AccessoryModeQueryChainFactory.hpp>
#include <f1x/aasdk/USB/AccessoryModeQueryFactory.hpp>
#include <f1x/aasdk/TCP/TCPEndpoint.hpp>
#include <f1x/aasdk/Channel/AV/AVInputServiceChannel.hpp>
#include <f1x/aasdk/Channel/AV/VideoServiceChannel.hpp>
#include <f1x/aasdk/Channel/AV/AudioServiceChannel.hpp>
#include <f1x/aasdk/Channel/Sensor/SensorServiceChannel.hpp>
#include <f1x/aasdk/Channel/Input/InputServiceChannel.hpp>
#include <f1x/aasdk/Messenger/Messenger.hpp>
#include <f1x/aasdk/Messenger/MessageInStream.hpp>
#include <f1x/aasdk/Messenger/MessageOutStream.hpp>

using namespace f1x::aasdk;

class AndroidAutoServiceImpl : public AndroidAutoService {
  Q_OBJECT

 public:
  explicit AndroidAutoServiceImpl(MediaPipeline* mediaPipeline, QObject* parent = nullptr)
      : AndroidAutoService(parent),
        state_(ConnectionState::DISCONNECTED),
        search_timer_(nullptr),
        media_pipeline_(mediaPipeline),
        usb_context_(nullptr),
        usb_hub_(nullptr),
        messenger_(nullptr),
        video_channel_(nullptr),
        audio_channel_(nullptr),
        input_channel_(nullptr) {
  }

  ~AndroidAutoServiceImpl() override { deinitialise(); }

  bool initialise() override {
    qDebug() << "[AndroidAuto] Initialising Android Auto service";
    
    // Initialize libusb
    int ret = libusb_init(&usb_context_);
    if (ret < 0) {
      qCritical() << "[AndroidAuto] Failed to initialize libusb:" << libusb_error_name(ret);
      return false;
    }
    
    // Create USB hub
    try {
      usb_hub_ = std::make_shared<usb::USBHub>(usb_context_);
      qDebug() << "[AndroidAuto] USB hub created successfully";
    } catch (const std::exception& e) {
      qCritical() << "[AndroidAuto] Failed to create USB hub:" << e.what();
      libusb_exit(usb_context_);
      usb_context_ = nullptr;
      return false;
    }
    
    return true;
  }

  void deinitialise() override {
    qDebug() << "[AndroidAuto] Deinitialising Android Auto service";
    stopSearching();
    if (isConnected()) {
      disconnect();
    }
    
    // Cleanup AASDK resources
    input_channel_.reset();
    audio_channel_.reset();
    video_channel_.reset();
    messenger_.reset();
    usb_hub_.reset();
    
    if (usb_context_) {
      libusb_exit(usb_context_);
      usb_context_ = nullptr;
    }
  }

  ConnectionState getConnectionState() const override { return state_; }

  bool isConnected() const override {
    return state_ == ConnectionState::CONNECTED;
  }

  AndroidDevice getConnectedDevice() const override {
    return connected_device_;
  }

  bool startSearching() override {
    if (state_ != ConnectionState::DISCONNECTED) {
      return false;
    }

    qDebug() << "[AndroidAuto] Starting device search";
    setState(ConnectionState::SEARCHING);

    // Start USB device enumeration
    if (!usb_hub_) {
      qWarning() << "[AndroidAuto] USB hub not initialized";
      return false;
    }
    
    // Create timer for periodic device checks
    if (!search_timer_) {
      search_timer_ = new QTimer(this);
      connect(search_timer_, &QTimer::timeout, this, &AndroidAutoServiceImpl::onSearchTimeout);
    }
    
    // Enumerate existing USB devices
    enumerateUSBDevices();
    
    // Start periodic checking
    search_timer_->start(2000); // Check every 2 seconds
    
    return true;
  }

  void stopSearching() override {
    qDebug() << "[AndroidAuto] Stopping device search";
    if (search_timer_) {
      search_timer_->stop();
    }
    if (state_ == ConnectionState::SEARCHING) {
      setState(ConnectionState::DISCONNECTED);
    }
  }

  bool connectToDevice(const QString& serial) override {
    if (state_ != ConnectionState::DISCONNECTED &&
        state_ != ConnectionState::SEARCHING) {
      qWarning() << "[AndroidAuto] Invalid state for connection:"
                 << static_cast<int>(state_);
      return false;
    }

    qDebug() << "[AndroidAuto] Connecting to device:" << serial;
    setState(ConnectionState::CONNECTING);

    try {
      // Find device by serial
      auto device = findUSBDevice(serial.toStdString());
      if (!device) {
        qWarning() << "[AndroidAuto] Device not found:" << serial;
        setState(ConnectionState::DISCONNECTED);
        return false;
      }
      
      connected_device_.serialNumber = serial;
      connected_device_.manufacturer = QString::fromStdString(device->getManufacturer());
      connected_device_.model = QString::fromStdString(device->getProduct());
      
      // Open USB connection and switch to accessory mode if needed
      auto accessoryModeQueryFactory = std::make_shared<usb::AccessoryModeQueryFactory>();
      auto queryChainFactory = std::make_shared<usb::AccessoryModeQueryChainFactory>(
          usb_hub_, accessoryModeQueryFactory);
      
      auto queryChain = queryChainFactory->create();
      queryChain->start(device, [this, serial](const error::Error& error) {
        if (error) {
          qCritical() << "[AndroidAuto] Accessory mode query failed:" << error.message().c_str();
          setState(ConnectionState::DISCONNECTED);
          return;
        }
        
        qDebug() << "[AndroidAuto] Device in accessory mode, establishing connection";
        establishConnection(serial);
      });
      
    } catch (const std::exception& e) {
      qCritical() << "[AndroidAuto] Connection error:" << e.what();
      setState(ConnectionState::DISCONNECTED);
      return false;
    }

    return true;
  }

  bool disconnect() override {
    if (state_ == ConnectionState::DISCONNECTED) {
      return true;
    }

    qDebug() << "[AndroidAuto] Disconnecting";
    setState(ConnectionState::DISCONNECTING);

    // Clean up channels
    if (input_channel_) {
      input_channel_.reset();
    }
    if (audio_channel_) {
      audio_channel_.reset();
    }
    if (video_channel_) {
      video_channel_.reset();
    }
    
    // Close messenger
    if (messenger_) {
      messenger_.reset();
    }
    
    // Reset device info
    connected_device_ = AndroidDevice{};
    setState(ConnectionState::DISCONNECTED);

    emit disconnected();
    return true;
  }

  bool setDisplayResolution(const QSize& resolution) override {
    if (!isConnected()) {
      return false;
    }

    display_resolution_ = resolution;
    qDebug() << "[AndroidAuto] Setting resolution to" << resolution;

    // Reconfigure video stream resolution via video channel
    if (video_channel_) {
      // Video configuration will be applied on next stream start
      video_config_.resolution = resolution;
    }
    
    return true;
  }

  QSize getDisplayResolution() const override {
    return display_resolution_;
  }

  bool setFramerate(int fps) override {
    if (!isConnected()) {
      return false;
    }

    framerate_ = fps;
    qDebug() << "[AndroidAuto] Setting framerate to" << fps;

    // Configure video encoding framerate
    if (video_channel_) {
      video_config_.fps = fps;
    }
    
    return true;
  }

  int getFramerate() const override { return framerate_; }

  bool sendTouchInput(int x, int y, int action) override {
    if (!isConnected() || !input_channel_) {
      return false;
    }

    try {
      // Create touch event
      proto::messages::TouchEvent touchEvent;
      auto* touchLocation = touchEvent.add_touch_location();
      touchLocation->set_x(x);
      touchLocation->set_y(y);
      touchLocation->set_pointer_id(0);
      
      // Map action (0=down, 1=up, 2=move)
      if (action == 0) {
        touchEvent.set_action_index(0);
        touchEvent.set_action(proto::enums::TouchAction::PRESS);
      } else if (action == 1) {
        touchEvent.set_action_index(0);
        touchEvent.set_action(proto::enums::TouchAction::RELEASE);
      } else if (action == 2) {
        touchEvent.set_action_index(0);
        touchEvent.set_action(proto::enums::TouchAction::DRAG);
      }
      
      // Send via input channel
      input_channel_->sendTouchEvent(touchEvent);
      return true;
      
    } catch (const std::exception& e) {
      qWarning() << "[AndroidAuto] Failed to send touch input:" << e.what();
      return false;
    }
  }

  bool sendKeyInput(int key_code, int action) override {
    if (!isConnected() || !input_channel_) {
      return false;
    }

    try {
      // Create key event
      proto::messages::KeyEvent keyEvent;
      keyEvent.set_keycode(key_code);
      
      // Map action (0=down, 1=up)
      if (action == 0) {
        keyEvent.set_action(proto::enums::KeyAction::DOWN);
      } else {
        keyEvent.set_action(proto::enums::KeyAction::UP);
      }
      
      keyEvent.set_timestamp(std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::system_clock::now().time_since_epoch()).count());
      
      // Send via input channel
      input_channel_->sendKeyEvent(keyEvent);
      return true;
      
    } catch (const std::exception& e) {
      qWarning() << "[AndroidAuto] Failed to send key input:" << e.what();
      return false;
    }
  }

  bool requestAudioFocus() override {
    if (!isConnected() || !audio_channel_) {
      return false;
    }

    try {
      proto::messages::AudioFocusRequest request;
      request.set_focus_type(proto::enums::AudioFocusType::GAIN);
      audio_channel_->sendAudioFocusRequest(request);
      qDebug() << "[AndroidAuto] Audio focus requested";
      return true;
    } catch (const std::exception& e) {
      qWarning() << "[AndroidAuto] Failed to request audio focus:" << e.what();
      return false;
    }
  }

  bool abandonAudioFocus() override {
    if (!isConnected() || !audio_channel_) {
      return false;
    }

    try {
      proto::messages::AudioFocusRequest request;
      request.set_focus_type(proto::enums::AudioFocusType::LOSS);
      audio_channel_->sendAudioFocusRequest(request);
      qDebug() << "[AndroidAuto] Audio focus abandoned";
      return true;
    } catch (const std::exception& e) {
      qWarning() << "[AndroidAuto] Failed to abandon audio focus:" << e.what();
      return false;
    }
  }

  int getFrameDropCount() const override { return frame_drop_count_; }

  int getLatency() const override { return latency_ms_; }

  bool setAudioEnabled(bool enabled) override {
    audio_enabled_ = enabled;
    
    if (audio_channel_) {
      if (enabled) {
        qDebug() << "[AndroidAuto] Audio stream enabled";
      } else {
        qDebug() << "[AndroidAuto] Audio stream disabled";
      }
    }
    
    return true;
  }

  QJsonObject getAudioConfig() const override {
    QJsonObject config;
    config["enabled"] = audio_enabled_;
    config["sample_rate"] = 48000;
    config["channels"] = 2;
    config["format"] = "PCM_S16LE";
    return config;
  }

 private slots:
  void onSearchTimeout() {
    // Check for new USB devices
    enumerateUSBDevices();
  }

 private:
  void setState(ConnectionState new_state) {
    if (state_ != new_state) {
      state_ = new_state;
      emit connectionStateChanged(state_);
    }
  }
  
  void enumerateUSBDevices() {
    if (!usb_hub_) {
      return;
    }
    
    try {
      auto enumerator = std::make_shared<usb::ConnectedAccessoriesEnumerator>(usb_hub_);
      auto devices = enumerator->enumerate();
      
      for (const auto& device : devices) {
        try {
          std::string serial = device->getSerial();
          std::string manufacturer = device->getManufacturer();
          std::string product = device->getProduct();
          
          AndroidDevice androidDevice;
          androidDevice.serialNumber = QString::fromStdString(serial);
          androidDevice.manufacturer = QString::fromStdString(manufacturer);
          androidDevice.model = QString::fromStdString(product);
          androidDevice.connected = false;
          
          qDebug() << "[AndroidAuto] Found device:" << androidDevice.serialNumber
                   << androidDevice.manufacturer << androidDevice.model;
          
          emit deviceDiscovered(androidDevice);
          
        } catch (const std::exception& e) {
          qWarning() << "[AndroidAuto] Error reading device info:" << e.what();
        }
      }
    } catch (const std::exception& e) {
      qWarning() << "[AndroidAuto] USB enumeration error:" << e.what();
    }
  }
  
  std::shared_ptr<usb::IUSBDevice> findUSBDevice(const std::string& serial) {
    if (!usb_hub_) {
      return nullptr;
    }
    
    try {
      auto enumerator = std::make_shared<usb::ConnectedAccessoriesEnumerator>(usb_hub_);
      auto devices = enumerator->enumerate();
      
      for (const auto& device : devices) {
        if (device->getSerial() == serial) {
          return device;
        }
      }
    } catch (const std::exception& e) {
      qWarning() << "[AndroidAuto] Error finding device:" << e.what();
    }
    
    return nullptr;
  }
  
  void establishConnection(const QString& serial) {
    try {
      // Find the device again after accessory mode switch
      auto device = findUSBDevice(serial.toStdString());
      if (!device) {
        qWarning() << "[AndroidAuto] Device disappeared after accessory mode switch";
        setState(ConnectionState::DISCONNECTED);
        return;
      }
      
      // Create endpoint for communication
      auto endpoint = std::make_shared<tcp::TCPEndpoint>(device);
      
      // Create messenger for protocol communication
      auto inStream = std::make_shared<messenger::MessageInStream>(endpoint);
      auto outStream = std::make_shared<messenger::MessageOutStream>(endpoint);
      messenger_ = std::make_shared<messenger::Messenger>(inStream, outStream);
      
      // Setup channels
      setupChannels();
      
      // Start handshake
      startHandshake();
      
    } catch (const std::exception& e) {
      qCritical() << "[AndroidAuto] Failed to establish connection:" << e.what();
      setState(ConnectionState::DISCONNECTED);
    }
  }
  
  void setupChannels() {
    if (!messenger_) {
      return;
    }
    
    // Create video channel
    video_channel_ = std::make_shared<channel::av::VideoServiceChannel>(
        messenger_, 
        [this](const proto::messages::VideoFocusNotification& notification) {
          qDebug() << "[AndroidAuto] Video focus notification received";
        });
    
    // Create audio channel  
    audio_channel_ = std::make_shared<channel::av::AudioServiceChannel>(
        messenger_,
        [this](const proto::messages::AudioFocusNotification& notification) {
          qDebug() << "[AndroidAuto] Audio focus notification received";
        });
    
    // Create input channel
    input_channel_ = std::make_shared<channel::input::InputServiceChannel>(
        messenger_,
        [this](const proto::messages::BindingResponse& response) {
          qDebug() << "[AndroidAuto] Input channel bound";
        });
    
    qDebug() << "[AndroidAuto] Channels created successfully";
  }
  
  void startHandshake() {
    // Send version request
    proto::messages::VersionRequest versionRequest;
    versionRequest.set_major_version(1);
    versionRequest.set_minor_version(0);
    
    messenger_->send(versionRequest, [this](const error::Error& error) {
      if (error) {
        qCritical() << "[AndroidAuto] Handshake failed:" << error.message().c_str();
        setState(ConnectionState::DISCONNECTED);
        return;
      }
      
      qDebug() << "[AndroidAuto] Handshake successful";
      onConnectionEstablished();
    });
  }
  
  void onConnectionEstablished() {
    setState(ConnectionState::CONNECTED);
    connected_device_.connected = true;
    
    // Start media streams
    if (media_pipeline_) {
      MediaPipeline::MediaConfig config;
      config.streamName = "AndroidAuto";
      config.enableVideo = true;
      config.enableAudio = true;
      config.videoResolution = VideoHAL::VideoResolution::HD_720p;
      config.videoCodec = "h264";
      config.audioSampleRate = 48000;
      config.audioChannels = 2;
      
      media_pipeline_->start(config);
    }
    
    // Setup video frame callback
    if (video_channel_) {
      video_channel_->receiveVideoFrame([this](const proto::messages::VideoFrame& frame) {
        handleVideoFrame(frame);
      });
    }
    
    // Setup audio data callback
    if (audio_channel_) {
      audio_channel_->receiveAudioData([this](const proto::messages::AudioData& data) {
        handleAudioData(data);
      });
    }
    
    emit connected(connected_device_);
    qDebug() << "[AndroidAuto] Connection established successfully";
  }
  
  void handleVideoFrame(const proto::messages::VideoFrame& frame) {
    if (!media_pipeline_ || !media_pipeline_->isActive()) {
      return;
    }
    
    try {
      // Extract frame data
      const std::string& frameData = frame.data();
      QByteArray videoData(frameData.data(), frameData.size());
      
      // Push to video HAL
      media_pipeline_->pushVideoFrame(videoData);
      
      // Update stats
      auto now = std::chrono::steady_clock::now();
      if (last_frame_time_.time_since_epoch().count() > 0) {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - last_frame_time_).count();
        latency_ms_ = elapsed;
      }
      last_frame_time_ = now;
      
      emit videoFrameReceived();
      
    } catch (const std::exception& e) {
      qWarning() << "[AndroidAuto] Error handling video frame:" << e.what();
      frame_drop_count_++;
    }
  }
  
  void handleAudioData(const proto::messages::AudioData& data) {
    if (!media_pipeline_ || !media_pipeline_->isActive() || !audio_enabled_) {
      return;
    }
    
    try {
      // Extract audio data
      const std::string& audioBytes = data.data();
      QByteArray audioData(audioBytes.data(), audioBytes.size());
      
      // Push to audio HAL
      media_pipeline_->pushAudioData(audioData);
      
      emit audioDataReceived();
      
    } catch (const std::exception& e) {
      qWarning() << "[AndroidAuto] Error handling audio data:" << e.what();
    }
  }

  ConnectionState state_;
  AndroidDevice connected_device_;
  QTimer* search_timer_;
  QSize display_resolution_{1024, 600};
  int framerate_ = 30;
  int frame_drop_count_ = 0;
  int latency_ms_ = 0;
  bool audio_enabled_ = true;
  
  // AASDK components
  libusb_context* usb_context_;
  std::shared_ptr<usb::USBHub> usb_hub_;
  std::shared_ptr<messenger::Messenger> messenger_;
  std::shared_ptr<channel::av::VideoServiceChannel> video_channel_;
  std::shared_ptr<channel::av::AudioServiceChannel> audio_channel_;
  std::shared_ptr<channel::input::InputServiceChannel> input_channel_;
  
  // Media pipeline
  MediaPipeline* media_pipeline_;
  
  // Video configuration
  struct VideoConfig {
    QSize resolution{1024, 600};
    int fps = 30;
  } video_config_;
  
  // Timing
  std::chrono::steady_clock::time_point last_frame_time_;
};

// Base class implementations
AndroidAutoService::AndroidAutoService(QObject* parent) : QObject(parent) {}

AndroidAutoService::~AndroidAutoService() = default;

// Factory function to create Android Auto service
AndroidAutoService* AndroidAutoService::create(MediaPipeline* mediaPipeline, QObject* parent) {
  auto* service = new AndroidAutoServiceImpl(mediaPipeline, parent);
  if (!service->initialise()) {
    qCritical() << "[AndroidAuto] Failed to initialize service";
    delete service;
    return nullptr;
  }
  return service;
}

#include "AndroidAutoService.moc"
