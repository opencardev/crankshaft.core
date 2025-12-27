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

#include "RealAndroidAutoService.h"

#include <QImage>
#include <QJsonObject>
#include <QTimer>

#include "../../hal/multimedia/AudioMixer.h"
#include "../../hal/multimedia/GStreamerVideoDecoder.h"
#include "../logging/Logger.h"
#include "ProtocolHelpers.h"

// AASDK includes
#include <aasdk/Channel/Bluetooth/BluetoothService.hpp>
#include <aasdk/Channel/Control/ControlServiceChannel.hpp>
#include <aasdk/Channel/InputSource/InputSourceService.hpp>
#include <aasdk/Channel/MediaSink/Audio/Channel/GuidanceAudioChannel.hpp>
#include <aasdk/Channel/MediaSink/Audio/Channel/MediaAudioChannel.hpp>
#include <aasdk/Channel/MediaSink/Audio/Channel/SystemAudioChannel.hpp>
#include <aasdk/Channel/MediaSink/Video/Channel/VideoChannel.hpp>
#include <aasdk/Channel/SensorSource/SensorSourceService.hpp>
#include <aasdk/Messenger/Cryptor.hpp>
#include <aasdk/Messenger/MessageInStream.hpp>
#include <aasdk/Messenger/MessageOutStream.hpp>
#include <aasdk/Messenger/Messenger.hpp>
#include <aasdk/TCP/TCPEndpoint.hpp>
#include <aasdk/TCP/TCPWrapper.hpp>
#include <aasdk/Transport/SSLWrapper.hpp>
#include <aasdk/Transport/TCPTransport.hpp>
#include <aasdk/Transport/USBTransport.hpp>
#include <aasdk/USB/AOAPDevice.hpp>
#include <aasdk/USB/AccessoryModeQueryChainFactory.hpp>
#include <aasdk/USB/AccessoryModeQueryFactory.hpp>
#include <aasdk/USB/USBHub.hpp>
#include <aasdk/USB/IAccessoryModeQueryChain.hpp>
#include <aasdk/USB/USBWrapper.hpp>
#include <boost/asio.hpp>

RealAndroidAutoService::RealAndroidAutoService(MediaPipeline* mediaPipeline, QObject* parent)
    : AndroidAutoService(parent), m_mediaPipeline(mediaPipeline) {
  // Create AASDK thread
  m_aasdkThread = std::make_unique<QThread>();
  m_aasdkThread->setObjectName("AASDKThread");
}

RealAndroidAutoService::~RealAndroidAutoService() {
  deinitialise();

  if (m_aasdkThread && m_aasdkThread->isRunning()) {
    m_aasdkThread->quit();
    m_aasdkThread->wait();
  }
}

void RealAndroidAutoService::configureTransport(const QMap<QString, QVariant>& settings) {
  QString mode = settings.value("connectionMode", "auto").toString().toLower();
  Logger::instance().info(
      QString("[RealAndroidAutoService] Configuring transport mode: %1").arg(mode));

  if (mode == "usb") {
    m_transportMode = TransportMode::USB;
    m_wirelessEnabled = false;
  } else if (mode == "wireless") {
    m_transportMode = TransportMode::Wireless;
    m_wirelessEnabled = true;
  } else {
    m_transportMode = TransportMode::Auto;
    m_wirelessEnabled = settings.value("wireless.enabled", false).toBool();
  }

  if (m_wirelessEnabled || m_transportMode == TransportMode::Wireless) {
    m_wirelessHost = settings.value("wireless.host", "").toString();
    m_wirelessPort = settings.value("wireless.port", 5277).toUInt();

    if (m_wirelessHost.isEmpty() && m_transportMode == TransportMode::Wireless) {
      Logger::instance().warning(
          "[RealAndroidAutoService] Wireless mode selected but no host configured.");
    } else if (!m_wirelessHost.isEmpty()) {
      Logger::instance().info(QString("[RealAndroidAutoService] Wireless AA configured: %1:%2")
                                  .arg(m_wirelessHost)
                                  .arg(m_wirelessPort));
    }
  }
}

bool RealAndroidAutoService::initialise() {
  if (m_isInitialised) {
    Logger::instance().warning("AndroidAutoService already initialised");
    return false;
  }

  try {
    setupAASDK();
    m_isInitialised = true;
    transitionToState(ConnectionState::DISCONNECTED);
    Logger::instance().info("AndroidAutoService initialised successfully");
    return true;
  } catch (const std::exception& e) {
    Logger::instance().error(QString("Failed to initialise AndroidAutoService: %1").arg(e.what()));
    emit errorOccurred(QString("Initialisation failed: %1").arg(e.what()));
    return false;
  }
}

void RealAndroidAutoService::deinitialise() {
  if (!m_isInitialised) {
    return;
  }

  stopSearching();
  if (isConnected()) {
    disconnect();
  }

  cleanupAASDK();
  m_isInitialised = false;
  transitionToState(ConnectionState::DISCONNECTED);
  Logger::instance().info("AndroidAutoService deinitialised");
}

void RealAndroidAutoService::setupAASDK() {
  // Create io_service
  m_ioService = std::make_shared<boost::asio::io_service>();

  // Create strand for channel operations
  m_strand = std::make_unique<boost::asio::io_service::strand>(*m_ioService);

  // Initialize libusb
  libusb_context* usbContext = nullptr;
  int ret = libusb_init(&usbContext);
  if (ret != 0) {
    Logger::instance().error(QString("Failed to initialize libusb: %1").arg(ret));
    throw std::runtime_error("libusb initialization failed");
  }

  // Create USB wrapper with libusb context
  m_usbWrapper = std::make_shared<aasdk::usb::USBWrapper>(usbContext);

  // Create query factories for AOAP device initialization
  m_queryFactory =
      std::make_shared<aasdk::usb::AccessoryModeQueryFactory>(*m_usbWrapper, *m_ioService);
  m_queryChainFactory = std::make_shared<aasdk::usb::AccessoryModeQueryChainFactory>(
      *m_usbWrapper, *m_ioService, *m_queryFactory);

  // Create USB hub for device hotplug detection
  m_usbHub =
      std::make_shared<aasdk::usb::USBHub>(*m_usbWrapper, *m_ioService, *m_queryChainFactory);

  // Start AASDK thread
  m_aasdkThread->start();

  // Integrate io_service with Qt event loop via periodic polling
  if (m_ioServiceTimer == nullptr) {
    m_ioServiceTimer = new QTimer(this);
    m_ioServiceTimer->setObjectName("AASDKIoServicePoller");
    connect(m_ioServiceTimer, &QTimer::timeout, this, [this]() {
      // Process Boost.Asio tasks
      if (m_ioService) {
        m_ioService->poll();
      }
      // Drive libusb event loop so hotplug and control transfers progress
      if (m_usbWrapper) {
        m_usbWrapper->handleEvents();
      }
    });
    m_ioServiceTimer->start(10);
  }

  Logger::instance().info("AASDK components initialised");
}

RealAndroidAutoService::TransportMode RealAndroidAutoService::getTransportMode() const {
  return m_transportMode;
}

bool RealAndroidAutoService::setupUSBTransport() {
  if (!m_usbHub || !m_ioService) {
    Logger::instance().error(
        "[RealAndroidAutoService] Cannot setup USB transport: components not ready");
    return false;
  }

  Logger::instance().info("[RealAndroidAutoService] Setting up USB transport...");
  // USB transport initialization is handled by USBHub in setupAASDK()
  // This method is called to explicitly select USB mode
  return true;
}

bool RealAndroidAutoService::setupTCPTransport(const QString& host, quint16 port) {
  if (host.isEmpty()) {
    Logger::instance().error("[RealAndroidAutoService] Cannot setup TCP transport: host is empty");
    return false;
  }

  if (!m_ioService) {
    Logger::instance().error(
        "[RealAndroidAutoService] Cannot setup TCP transport: io_service not ready");
    return false;
  }

  Logger::instance().info(
      QString("[RealAndroidAutoService] Setting up TCP transport to %1:%2").arg(host).arg(port));

  try {
    // Create TCP wrapper
    auto tcpWrapper = std::make_shared<aasdk::tcp::TCPWrapper>();

    // Create socket
    auto socket = std::make_shared<boost::asio::ip::tcp::socket>(*m_ioService);

    // Connect to phone (synchronous for now)
    boost::system::error_code ec =
        tcpWrapper->connect(*socket, host.toStdString(), static_cast<uint16_t>(port));

    if (ec) {
      Logger::instance().error(QString("[RealAndroidAutoService] Failed to connect to %1:%2 - %3")
                                   .arg(host)
                                   .arg(port)
                                   .arg(QString::fromStdString(ec.message())));
      return false;
    }

    // Create TCP endpoint using the wrapper and socket
    auto tcpEndpoint = std::make_shared<aasdk::tcp::TCPEndpoint>(*tcpWrapper, socket);

    // Create TCP transport from the endpoint
    m_transport = std::make_shared<aasdk::transport::TCPTransport>(*m_ioService, tcpEndpoint);

    Logger::instance().info(
        QString("[RealAndroidAutoService] TCP transport connected to %1:%2").arg(host).arg(port));

    // Setup channels with the TCP transport
    setupChannelsWithTransport();

    return true;
  } catch (const std::exception& e) {
    Logger::instance().error(
        QString("[RealAndroidAutoService] Exception setting up TCP transport: %1").arg(e.what()));
    return false;
  }
}

void RealAndroidAutoService::setupChannels() {
  if (!m_aoapDevice || !m_ioService) {
    Logger::instance().error("Cannot setup channels: device or io_service not ready");
    return;
  }

  try {
    // Create transport layer
    m_transport = std::make_shared<aasdk::transport::USBTransport>(*m_ioService, m_aoapDevice);

    // Create SSL/encryption layer
    auto sslWrapper = std::make_shared<aasdk::transport::SSLWrapper>();
    m_cryptor = std::make_shared<aasdk::messenger::Cryptor>(std::move(sslWrapper));
    m_cryptor->init();

    // Create messenger
    auto messageInStream =
        std::make_shared<aasdk::messenger::MessageInStream>(*m_ioService, m_transport, m_cryptor);
    auto messageOutStream =
        std::make_shared<aasdk::messenger::MessageOutStream>(*m_ioService, m_transport, m_cryptor);
    m_messenger = std::make_shared<aasdk::messenger::Messenger>(
        *m_ioService, std::move(messageInStream), std::move(messageOutStream));

    // Create control channel (required)
    m_controlChannel =
        std::make_shared<aasdk::channel::control::ControlServiceChannel>(*m_strand, m_messenger);

    // Create video channel
    if (m_channelConfig.videoEnabled) {
      m_videoChannel = std::make_shared<aasdk::channel::mediasink::video::channel::VideoChannel>(
          *m_strand, m_messenger);
      Logger::instance().info("Video channel enabled");
    }

    // Create media audio channel
    if (m_channelConfig.mediaAudioEnabled) {
      m_mediaAudioChannel =
          std::make_shared<aasdk::channel::mediasink::audio::channel::MediaAudioChannel>(
              *m_strand, m_messenger);
      Logger::instance().info("Media audio channel enabled");
    }

    // Create system audio channel
    if (m_channelConfig.systemAudioEnabled) {
      m_systemAudioChannel =
          std::make_shared<aasdk::channel::mediasink::audio::channel::SystemAudioChannel>(
              *m_strand, m_messenger);
      Logger::instance().info("System audio channel enabled");
    }

    // Create speech audio channel (using GuidanceAudioChannel)
    if (m_channelConfig.speechAudioEnabled) {
      m_speechAudioChannel =
          std::make_shared<aasdk::channel::mediasink::audio::channel::GuidanceAudioChannel>(
              *m_strand, m_messenger);
      Logger::instance().info("Speech audio channel enabled");
    }

    // Create input channel
    if (m_channelConfig.inputEnabled) {
      m_inputChannel =
          std::make_shared<aasdk::channel::inputsource::InputSourceService>(*m_strand, m_messenger);
      Logger::instance().info("Input channel enabled");
    }

    // Create sensor channel
    if (m_channelConfig.sensorEnabled) {
      m_sensorChannel = std::make_shared<aasdk::channel::sensorsource::SensorSourceService>(
          *m_strand, m_messenger);
      Logger::instance().info("Sensor channel enabled");
    }

    // Create bluetooth channel
    if (m_channelConfig.bluetoothEnabled) {
      m_bluetoothChannel =
          std::make_shared<aasdk::channel::bluetooth::BluetoothService>(*m_strand, m_messenger);
      Logger::instance().info("Bluetooth channel enabled");
    }

    // Initialize video decoder
    if (m_channelConfig.videoEnabled) {
      m_videoDecoder = new GStreamerVideoDecoder(this);

      IVideoDecoder::DecoderConfig decoderConfig;
      decoderConfig.codec = IVideoDecoder::CodecType::H264;
      decoderConfig.width = m_resolution.width();
      decoderConfig.height = m_resolution.height();
      decoderConfig.fps = m_fps;
      decoderConfig.outputFormat = IVideoDecoder::PixelFormat::RGBA;
      decoderConfig.hardwareAcceleration = true;

      if (m_videoDecoder->initialize(decoderConfig)) {
        connect(m_videoDecoder, &IVideoDecoder::frameDecoded, this,
                [this](int width, int height, const uint8_t* data, int size) {
                  emit videoFrameReady(width, height, data, size);
                });

        connect(m_videoDecoder, &IVideoDecoder::errorOccurred, this, [](const QString& error) {
          Logger::instance().error("Video decoder error: " + error);
        });

        Logger::instance().info(
            QString("Video decoder initialized: %1").arg(m_videoDecoder->getDecoderName()));
      } else {
        Logger::instance().error("Failed to initialize video decoder");
        delete m_videoDecoder;
        m_videoDecoder = nullptr;
      }
    }

    // Initialize audio mixer
    if (m_channelConfig.mediaAudioEnabled || m_channelConfig.systemAudioEnabled ||
        m_channelConfig.speechAudioEnabled) {
      m_audioMixer = new AudioMixer(this);

      IAudioMixer::AudioFormat masterFormat;
      masterFormat.sampleRate = 48000;
      masterFormat.channels = 2;
      masterFormat.bitsPerSample = 16;

      if (m_audioMixer->initialize(masterFormat)) {
        // Add media audio channel (48kHz stereo)
        if (m_channelConfig.mediaAudioEnabled) {
          IAudioMixer::ChannelConfig mediaConfig;
          mediaConfig.id = IAudioMixer::ChannelId::MEDIA;
          mediaConfig.volume = 0.8f;
          mediaConfig.priority = 1;
          mediaConfig.format = masterFormat;
          m_audioMixer->addChannel(mediaConfig);
        }

        // Add system audio channel (16kHz mono)
        if (m_channelConfig.systemAudioEnabled) {
          IAudioMixer::ChannelConfig systemConfig;
          systemConfig.id = IAudioMixer::ChannelId::SYSTEM;
          systemConfig.volume = 1.0f;
          systemConfig.priority = 2;
          systemConfig.format = {16000, 1, 16};
          m_audioMixer->addChannel(systemConfig);
        }

        // Add speech audio channel (16kHz mono)
        if (m_channelConfig.speechAudioEnabled) {
          IAudioMixer::ChannelConfig speechConfig;
          speechConfig.id = IAudioMixer::ChannelId::SPEECH;
          speechConfig.volume = 1.0f;
          speechConfig.priority = 3;
          speechConfig.format = {16000, 1, 16};
          m_audioMixer->addChannel(speechConfig);
        }

        // Connect mixed audio output
        connect(m_audioMixer, &IAudioMixer::audioMixed, this,
                [this](const QByteArray& mixedData) { emit audioDataReady(mixedData); });

        connect(m_audioMixer, &IAudioMixer::errorOccurred, this, [](const QString& error) {
          Logger::instance().error("Audio mixer error: " + error);
        });

        Logger::instance().info("Audio mixer initialized with multiple channels");
      } else {
        Logger::instance().error("Failed to initialize audio mixer");
        delete m_audioMixer;
        m_audioMixer = nullptr;
      }
    }

    Logger::instance().info("All enabled channels created successfully");
  } catch (const std::exception& e) {
    Logger::instance().error(QString("Failed to setup channels: %1").arg(e.what()));
    emit errorOccurred(QString("Channel setup failed: %1").arg(e.what()));
  }
}

void RealAndroidAutoService::setupChannelsWithTransport() {
  if (!m_transport || !m_ioService) {
    Logger::instance().error("Cannot setup channels: transport or io_service not ready");
    return;
  }

  try {
    // Create SSL/encryption layer
    auto sslWrapper = std::make_shared<aasdk::transport::SSLWrapper>();
    m_cryptor = std::make_shared<aasdk::messenger::Cryptor>(std::move(sslWrapper));
    m_cryptor->init();

    // Create messenger
    auto messageInStream =
        std::make_shared<aasdk::messenger::MessageInStream>(*m_ioService, m_transport, m_cryptor);
    auto messageOutStream =
        std::make_shared<aasdk::messenger::MessageOutStream>(*m_ioService, m_transport, m_cryptor);
    m_messenger = std::make_shared<aasdk::messenger::Messenger>(
        *m_ioService, std::move(messageInStream), std::move(messageOutStream));

    // Create control channel (required)
    m_controlChannel =
        std::make_shared<aasdk::channel::control::ControlServiceChannel>(*m_strand, m_messenger);

    // Create video channel
    if (m_channelConfig.videoEnabled) {
      m_videoChannel = std::make_shared<aasdk::channel::mediasink::video::channel::VideoChannel>(
          *m_strand, m_messenger);
      Logger::instance().info("Video channel enabled (TCP)");
    }

    // Create media audio channel
    if (m_channelConfig.mediaAudioEnabled) {
      m_mediaAudioChannel =
          std::make_shared<aasdk::channel::mediasink::audio::channel::MediaAudioChannel>(
              *m_strand, m_messenger);
      Logger::instance().info("Media audio channel enabled (TCP)");
    }

    // Create system audio channel
    if (m_channelConfig.systemAudioEnabled) {
      m_systemAudioChannel =
          std::make_shared<aasdk::channel::mediasink::audio::channel::SystemAudioChannel>(
              *m_strand, m_messenger);
      Logger::instance().info("System audio channel enabled (TCP)");
    }

    // Create speech audio channel (using GuidanceAudioChannel)
    if (m_channelConfig.speechAudioEnabled) {
      m_speechAudioChannel =
          std::make_shared<aasdk::channel::mediasink::audio::channel::GuidanceAudioChannel>(
              *m_strand, m_messenger);
      Logger::instance().info("Speech audio channel enabled (TCP)");
    }

    // Create input channel
    if (m_channelConfig.inputEnabled) {
      m_inputChannel =
          std::make_shared<aasdk::channel::inputsource::InputSourceService>(*m_strand, m_messenger);
      Logger::instance().info("Input channel enabled (TCP)");
    }

    // Create sensor channel
    if (m_channelConfig.sensorEnabled) {
      m_sensorChannel = std::make_shared<aasdk::channel::sensorsource::SensorSourceService>(
          *m_strand, m_messenger);
      Logger::instance().info("Sensor channel enabled (TCP)");
    }

    // Create bluetooth channel
    if (m_channelConfig.bluetoothEnabled) {
      m_bluetoothChannel =
          std::make_shared<aasdk::channel::bluetooth::BluetoothService>(*m_strand, m_messenger);
      Logger::instance().info("Bluetooth channel enabled (TCP)");
    }

    // Initialize video decoder
    if (m_channelConfig.videoEnabled) {
      m_videoDecoder = new GStreamerVideoDecoder(this);

      IVideoDecoder::DecoderConfig decoderConfig;
      decoderConfig.codec = IVideoDecoder::CodecType::H264;
      decoderConfig.width = m_resolution.width();
      decoderConfig.height = m_resolution.height();
      decoderConfig.fps = m_fps;
      decoderConfig.outputFormat = IVideoDecoder::PixelFormat::RGBA;
      decoderConfig.hardwareAcceleration = true;

      if (m_videoDecoder->initialize(decoderConfig)) {
        connect(m_videoDecoder, &IVideoDecoder::frameDecoded, this,
                [this](int width, int height, const uint8_t* data, int size) {
                  emit videoFrameReady(width, height, data, size);
                });

        connect(m_videoDecoder, &IVideoDecoder::errorOccurred, this, [](const QString& error) {
          Logger::instance().error("Video decoder error: " + error);
        });

        Logger::instance().info(
            QString("Video decoder initialized: %1").arg(m_videoDecoder->getDecoderName()));
      } else {
        Logger::instance().error("Failed to initialize video decoder");
        delete m_videoDecoder;
        m_videoDecoder = nullptr;
      }
    }

    // Initialize audio mixer
    if (m_channelConfig.mediaAudioEnabled || m_channelConfig.systemAudioEnabled ||
        m_channelConfig.speechAudioEnabled) {
      m_audioMixer = new AudioMixer(this);

      IAudioMixer::AudioFormat masterFormat;
      masterFormat.sampleRate = 48000;
      masterFormat.channels = 2;
      masterFormat.bitsPerSample = 16;

      if (m_audioMixer->initialize(masterFormat)) {
        // Add media audio channel (48kHz stereo)
        if (m_channelConfig.mediaAudioEnabled) {
          IAudioMixer::ChannelConfig mediaConfig;
          mediaConfig.id = IAudioMixer::ChannelId::MEDIA;
          mediaConfig.volume = 0.8f;
          mediaConfig.priority = 1;
          mediaConfig.format = masterFormat;
          m_audioMixer->addChannel(mediaConfig);
        }

        // Add system audio channel (16kHz mono)
        if (m_channelConfig.systemAudioEnabled) {
          IAudioMixer::ChannelConfig systemConfig;
          systemConfig.id = IAudioMixer::ChannelId::SYSTEM;
          systemConfig.volume = 1.0f;
          systemConfig.priority = 2;
          systemConfig.format = {16000, 1, 16};
          m_audioMixer->addChannel(systemConfig);
        }

        // Add speech audio channel (16kHz mono)
        if (m_channelConfig.speechAudioEnabled) {
          IAudioMixer::ChannelConfig speechConfig;
          speechConfig.id = IAudioMixer::ChannelId::SPEECH;
          speechConfig.volume = 1.0f;
          speechConfig.priority = 3;
          speechConfig.format = {16000, 1, 16};
          m_audioMixer->addChannel(speechConfig);
        }

        // Connect mixed audio output
        connect(m_audioMixer, &IAudioMixer::audioMixed, this,
                [this](const QByteArray& mixedData) { emit audioDataReady(mixedData); });

        connect(m_audioMixer, &IAudioMixer::errorOccurred, this, [](const QString& error) {
          Logger::instance().error("Audio mixer error: " + error);
        });

        Logger::instance().info("Audio mixer initialized with multiple channels");
      } else {
        Logger::instance().error("Failed to initialize audio mixer");
        delete m_audioMixer;
        m_audioMixer = nullptr;
      }
    }

    Logger::instance().info("All enabled channels created successfully (TCP)");
  } catch (const std::exception& e) {
    Logger::instance().error(QString("Failed to setup channels (TCP): %1").arg(e.what()));
    emit errorOccurred(QString("Channel setup failed (TCP): %1").arg(e.what()));
  }
}

void RealAndroidAutoService::cleanupAASDK() {
  // Clean up channels first
  cleanupChannels();

  // Stop io_service poller
  if (m_ioServiceTimer) {
    m_ioServiceTimer->stop();
    m_ioServiceTimer->deleteLater();
    m_ioServiceTimer = nullptr;
  }

  // Stop USB hub
  if (m_usbHub) {
    m_usbHub->cancel();
    m_usbHub.reset();
  }

  // Clean up messenger
  if (m_messenger) {
    m_messenger->stop();
    m_messenger.reset();
  }

  // Clean up AOAP device
  if (m_aoapDevice) {
    m_aoapDevice.reset();
  }

  // Clean up USB wrapper
  if (m_usbWrapper) {
    m_usbWrapper.reset();
  }

  // Stop io_service
  if (m_ioService) {
    m_ioService->stop();
    m_ioService.reset();
  }

  Logger::instance().info("AASDK components cleaned up");
}

void RealAndroidAutoService::cleanupChannels() {
  // Cleanup multimedia components
  if (m_videoDecoder) {
    m_videoDecoder->deinitialize();
    delete m_videoDecoder;
    m_videoDecoder = nullptr;
    Logger::instance().info("Video decoder cleaned up");
  }

  if (m_audioMixer) {
    m_audioMixer->deinitialize();
    delete m_audioMixer;
    m_audioMixer = nullptr;
    Logger::instance().info("Audio mixer cleaned up");
  }

  // Reset all channel pointers
  m_videoChannel.reset();
  m_mediaAudioChannel.reset();
  m_systemAudioChannel.reset();
  m_speechAudioChannel.reset();
  m_inputChannel.reset();
  m_sensorChannel.reset();
  m_bluetoothChannel.reset();
  m_controlChannel.reset();

  m_transport.reset();
  m_cryptor.reset();

  Logger::instance().info("Channels cleaned up");
}

bool RealAndroidAutoService::startSearching() {
  if (!m_isInitialised) {
    Logger::instance().error("Cannot start searching: service not initialised");
    return false;
  }

  if (m_state != ConnectionState::DISCONNECTED) {
    Logger::instance().warning("Already searching or connected");
    return false;
  }

  // If wireless mode is configured, try TCP connection instead of USB search
  if (m_transportMode == TransportMode::Wireless || m_wirelessEnabled) {
    if (m_wirelessHost.isEmpty()) {
      Logger::instance().error(
          "[RealAndroidAutoService] Cannot start wireless search: no host configured");
      return false;
    }

    transitionToState(ConnectionState::CONNECTING);
    Logger::instance().info(
        QString("[RealAndroidAutoService] Starting wireless connection to %1:%2")
            .arg(m_wirelessHost)
            .arg(m_wirelessPort));

    if (setupTCPTransport(m_wirelessHost, m_wirelessPort)) {
      Logger::instance().info("[RealAndroidAutoService] Wireless connection established");
      transitionToState(ConnectionState::CONNECTED);
      emit connected(m_device);
      return true;
    } else {
      Logger::instance().error(QString("[RealAndroidAutoService] Failed to connect to %1:%2")
                                   .arg(m_wirelessHost)
                                   .arg(m_wirelessPort));
      transitionToState(ConnectionState::DISCONNECTED);
      return false;
    }
  }

  // USB mode: start USB hub to detect devices
  transitionToState(ConnectionState::SEARCHING);

  // Start USB hub to detect devices
  auto promise = aasdk::usb::IUSBHub::Promise::defer(*m_ioService);
  promise->then(
      [this](aasdk::usb::DeviceHandle deviceHandle) {
        Logger::instance().info("Device connected, creating AOAP transport");

        try {
          // Create AOAP device from handle
          m_aoapDevice =
              aasdk::usb::AOAPDevice::create(*m_usbWrapper, *m_ioService, std::move(deviceHandle));

          // Build transport/messenger and channels
          transitionToState(ConnectionState::CONNECTING);
          setupChannels();

          // Mark connection established
          handleConnectionEstablished();
        } catch (const std::exception& e) {
          Logger::instance().error(QString("Failed to initialise AOAP device: %1").arg(e.what()));
          transitionToState(ConnectionState::DISCONNECTED);
        }
      },
      [this](const aasdk::error::Error& error) {
        Logger::instance().error(
            QString("USB hub error: %1").arg(QString::fromStdString(error.what())));
        transitionToState(ConnectionState::DISCONNECTED);
      });
  m_usbHub->start(std::move(promise));

  // Start a periodic check for connected devices (fallback if hotplug doesn't work)
  if (m_deviceDetectionTimer == nullptr) {
    m_deviceDetectionTimer = new QTimer(this);
    m_deviceDetectionTimer->setObjectName("AADeviceDetectionTimer");
    connect(m_deviceDetectionTimer, &QTimer::timeout, this, &RealAndroidAutoService::checkForConnectedDevices);
    m_deviceDetectionTimer->start(1000);  // Check every second
    Logger::instance().info("Started periodic device detection timer (fallback for hotplug)");
  }

  Logger::instance().info("Started searching for Android Auto devices");
  return true;
}

void RealAndroidAutoService::stopSearching() {
  if (m_state == ConnectionState::SEARCHING) {
    m_usbHub->cancel();
    
    if (m_deviceDetectionTimer) {
      m_deviceDetectionTimer->stop();
    }
    
    transitionToState(ConnectionState::DISCONNECTED);
    Logger::instance().info("Stopped searching for devices");
  }
}

bool RealAndroidAutoService::connectToDevice(const QString& serial) {
  if (!m_isInitialised) {
    Logger::instance().error("Cannot connect: service not initialised");
    return false;
  }

  if (serial != m_device.serialNumber) {
    Logger::instance().error(QString("Unknown device: %1").arg(serial));
    emit errorOccurred("Unknown device: " + serial);
    return false;
  }

  transitionToState(ConnectionState::CONNECTING);
  Logger::instance().info(QString("Connecting to device: %1").arg(serial));

  // Connection will be handled by AOAP device setup
  // This is initiated from onUSBHotplug callback

  return true;
}

bool RealAndroidAutoService::disconnect() {
  if (!isConnected() && m_state != ConnectionState::CONNECTING) {
    return false;
  }

  transitionToState(ConnectionState::DISCONNECTING);
  Logger::instance().info("Disconnecting from device");

  // Stop messenger
  if (m_messenger) {
    m_messenger->stop();
  }

  // Release AOAP device
  if (m_aoapDevice) {
    m_aoapDevice.reset();
  }

  m_device.connected = false;
  transitionToState(ConnectionState::DISCONNECTED);
  emit disconnected();

  return true;
}

bool RealAndroidAutoService::setDisplayResolution(const QSize& resolution) {
  if (resolution.width() <= 0 || resolution.height() <= 0) {
    Logger::instance().error("Invalid resolution");
    return false;
  }

  m_resolution = resolution;
  Logger::instance().info(
      QString("Display resolution set to %1x%2").arg(resolution.width()).arg(resolution.height()));

  return true;
}

bool RealAndroidAutoService::setFramerate(int fps) {
  if (fps <= 0 || fps > 60) {
    Logger::instance().error("Invalid framerate");
    return false;
  }

  m_fps = fps;
  Logger::instance().info(QString("Framerate set to %1").arg(fps));
  return true;
}

bool RealAndroidAutoService::sendTouchInput(int x, int y, int action) {
  if (!isConnected() || !m_inputChannel) {
    Logger::instance().warning("Cannot send touch input: not connected or input channel disabled");
    return false;
  }

  try {
    using namespace crankshaft::protocol;

    // Normalize coordinates to 0-1 range
    float normalizedX = static_cast<float>(x) / m_resolution.width();
    float normalizedY = static_cast<float>(y) / m_resolution.height();

    // Map action (0=DOWN, 1=UP, 2=MOVE)
    TouchAction touchAction;
    if (action == 0) {
      touchAction = TouchAction::ACTION_DOWN;
    } else if (action == 1) {
      touchAction = TouchAction::ACTION_UP;
    } else {
      touchAction = TouchAction::ACTION_MOVED;
    }

    auto data = createTouchInputReport(normalizedX, normalizedY, touchAction);

    auto promise = aasdk::channel::SendPromise::defer(*m_strand);
    promise->then(
        []() {
          // Success - touch input sent
        },
        [](const aasdk::error::Error& error) {
          Logger::instance().warning(
              QString("Failed to send touch input: %1").arg(QString::fromStdString(error.what())));
        });

    m_inputChannel->sendInputReport(data, std::move(promise));

    Logger::instance().debug(QString("Touch input sent: x=%1, y=%2, action=%3")
                                 .arg(normalizedX)
                                 .arg(normalizedY)
                                 .arg(action));

    return true;
  } catch (const std::exception& e) {
    Logger::instance().error(QString("Failed to send touch input: %1").arg(e.what()));
    return false;
  }
}

bool RealAndroidAutoService::sendKeyInput(int key_code, int action) {
  if (!isConnected() || !m_inputChannel) {
    Logger::instance().warning("Cannot send key input: not connected or input channel disabled");
    return false;
  }

  try {
    using namespace crankshaft::protocol;

    // Map action (0=DOWN, 1=UP)
    KeyAction keyAction = (action == 0) ? KeyAction::ACTION_DOWN : KeyAction::ACTION_UP;

    auto data = createKeyInputReport(key_code, keyAction);

    auto promise = aasdk::channel::SendPromise::defer(*m_strand);
    promise->then(
        []() {
          // Success - key input sent
        },
        [](const aasdk::error::Error& error) {
          Logger::instance().warning(
              QString("Failed to send key input: %1").arg(QString::fromStdString(error.what())));
        });

    m_inputChannel->sendInputReport(data, std::move(promise));

    Logger::instance().debug(
        QString("Key input sent: code=%1, action=%2").arg(key_code).arg(action));

    return true;
  } catch (const std::exception& e) {
    Logger::instance().error(QString("Failed to send key input: %1").arg(e.what()));
    return false;
  }
}

bool RealAndroidAutoService::requestAudioFocus() {
  if (!isConnected() || !m_controlChannel) {
    Logger::instance().warning("Cannot request audio focus: not connected");
    return false;
  }

  try {
    using namespace crankshaft::protocol;

    auto data = createAudioFocusNotification(AudioFocusState::GAIN);

    auto promise = aasdk::channel::SendPromise::defer(*m_strand);
    promise->then([]() { Logger::instance().info("Audio focus granted to Android Auto"); },
                  [](const aasdk::error::Error& error) {
                    Logger::instance().warning(QString("Failed to request audio focus: %1")
                                                   .arg(QString::fromStdString(error.what())));
                  });

    m_controlChannel->sendAudioFocusResponse(data, std::move(promise));

    return true;
  } catch (const std::exception& e) {
    Logger::instance().error(QString("Failed to request audio focus: %1").arg(e.what()));
    return false;
  }
}

bool RealAndroidAutoService::abandonAudioFocus() {
  if (!isConnected() || !m_controlChannel) {
    Logger::instance().warning("Cannot abandon audio focus: not connected");
    return false;
  }

  try {
    using namespace crankshaft::protocol;

    auto data = createAudioFocusNotification(AudioFocusState::LOSS);

    auto promise = aasdk::channel::SendPromise::defer(*m_strand);
    promise->then([]() { Logger::instance().info("Audio focus removed from Android Auto"); },
                  [](const aasdk::error::Error& error) {
                    Logger::instance().warning(QString("Failed to abandon audio focus: %1")
                                                   .arg(QString::fromStdString(error.what())));
                  });

    m_controlChannel->sendAudioFocusResponse(data, std::move(promise));

    return true;
  } catch (const std::exception& e) {
    Logger::instance().error(QString("Failed to abandon audio focus: %1").arg(e.what()));
    return false;
  }
}

bool RealAndroidAutoService::setAudioEnabled(bool enabled) {
  m_audioEnabled = enabled;
  Logger::instance().info(QString("Audio %1").arg(enabled ? "enabled" : "disabled"));
  return true;
}

void RealAndroidAutoService::setChannelConfig(const ChannelConfig& config) {
  bool needsReconnect =
      isConnected() && (m_channelConfig.videoEnabled != config.videoEnabled ||
                        m_channelConfig.mediaAudioEnabled != config.mediaAudioEnabled ||
                        m_channelConfig.systemAudioEnabled != config.systemAudioEnabled ||
                        m_channelConfig.speechAudioEnabled != config.speechAudioEnabled ||
                        m_channelConfig.inputEnabled != config.inputEnabled ||
                        m_channelConfig.sensorEnabled != config.sensorEnabled ||
                        m_channelConfig.bluetoothEnabled != config.bluetoothEnabled);

  m_channelConfig = config;
  Logger::instance().info("Channel configuration updated");

  if (needsReconnect) {
    Logger::instance().info("Channel config changed while connected - reconnection required");
    emit errorOccurred("Channel configuration changed. Please reconnect.");
  }
}

QJsonObject RealAndroidAutoService::getAudioConfig() const {
  QJsonObject config;
  // TODO: Get actual audio config from AASDK
  config["sampleRate"] = 48000;
  config["channels"] = 2;
  config["bitsPerSample"] = 16;
  config["codec"] = "PCM";
  return config;
}

void RealAndroidAutoService::onUSBHotplug(bool connected) {
  Logger::instance().info(
      QString("USB hotplug event: %1").arg(connected ? "connected" : "disconnected"));

  if (connected) {
    handleDeviceDetected();
  } else {
    handleDeviceRemoved();
  }
}

void RealAndroidAutoService::handleDeviceDetected() {
  // Device found, populate device info
  m_device.serialNumber = "AA_DEVICE_REAL";  // TODO: Get from AOAP
  m_device.manufacturer = "Unknown";
  m_device.model = "Android Device";
  m_device.androidVersion = "Unknown";
  m_device.connected = false;
  m_device.projectionMode = ProjectionMode::PROJECTION;

  emit deviceFound(m_device);
}

void RealAndroidAutoService::handleDeviceRemoved() {
  if (isConnected()) {
    disconnect();
  }
}

void RealAndroidAutoService::handleConnectionEstablished() {
  // Setup channels after device is connected
  setupChannels();

  m_device.connected = true;
  transitionToState(ConnectionState::CONNECTED);
  emit connected(m_device);
  Logger::instance().info("Android Auto connection established");
}

void RealAndroidAutoService::handleConnectionLost() {
  if (isConnected()) {
    disconnect();
  }
}

void RealAndroidAutoService::checkForConnectedDevices() {
  if (!m_usbWrapper || m_state != ConnectionState::SEARCHING) {
    return;
  }

  try {
    aasdk::usb::DeviceListHandle listHandle;
    auto count = m_usbWrapper->getDeviceList(listHandle);
    
    if (count < 0 || !listHandle) {
      Logger::instance().debug("[RealAndroidAutoService] USB device list error");
      return;
    }

    for (auto* dev : *listHandle) {
      libusb_device_descriptor desc{};
      if (m_usbWrapper->getDeviceDescriptor(dev, desc) == 0) {
        // Look for Google devices (vendor ID 0x18D1)
        if (desc.idVendor == 0x18D1) {
          Logger::instance().info(
              QString("[RealAndroidAutoService] Found Google device: vid=0x%1 pid=0x%2")
                  .arg(QString::asprintf("%04x", desc.idVendor))
                  .arg(QString::asprintf("%04x", desc.idProduct)));

          // If it's already in AOAP mode, let USBHub handle it
          if (desc.idProduct == 0x2D00 || desc.idProduct == 0x2D01) {
            Logger::instance().info("[RealAndroidAutoService] ✓ Device in AOAP mode! Forwarding to USBHub...");
            return;  // Let USBHub promise handle this
          }

          // Skip if AOAP negotiation already in progress
          if (m_aoapInProgress) {
            Logger::instance().debug("[RealAndroidAutoService] AOAP already in progress, skipping");
            return;
          }

          // Respect maximum attempt limit to avoid tight retry loops
          if (m_aoapAttempts >= m_aoapMaxAttempts) {
            Logger::instance().warning(QString("[RealAndroidAutoService] Skipping AOAP attempt: reached max attempts (%1)")
                                         .arg(m_aoapMaxAttempts));
            return;
          }

          // Try to open device and initiate AOAP negotiation
          aasdk::usb::DeviceHandle handle;
          int openResult = m_usbWrapper->open(dev, handle);
          Logger::instance().info(QString("[RealAndroidAutoService] Open device result: %1").arg(openResult));
          
          if (openResult == 0 && handle) {
            Logger::instance().info("[RealAndroidAutoService] Opened device for AOAP negotiation");
            
            if (m_queryChainFactory && m_ioService) {
              m_aoapInProgress = true;
              Logger::instance().info("[RealAndroidAutoService] Creating AccessoryModeQueryChain...");
              auto chain = m_queryChainFactory->create();
              // Use io_service directly, not strand, for the promise
              auto aoapPromise = aasdk::usb::IAccessoryModeQueryChain::Promise::defer(*m_ioService);
              
              auto onSuccess = [this](aasdk::usb::DeviceHandle devHandle) {
                m_aoapInProgress = false;
                m_aoapAttempts = 0; // reset attempts on success
                if (m_aoapRetryResetTimer) {
                  m_aoapRetryResetTimer->stop();
                }
                Logger::instance().info("[RealAndroidAutoService] AOAP query chain completed (success)");
                // Device may or may not have switched; restart timer to check
                if (m_deviceDetectionTimer && m_state == ConnectionState::SEARCHING) {
                  QTimer::singleShot(2000, this, [this]() {
                    if (m_deviceDetectionTimer && m_state == ConnectionState::SEARCHING) {
                      m_deviceDetectionTimer->start();
                    }
                  });
                }
              };
              
              auto onError = [this](const aasdk::error::Error& error) {
                m_aoapInProgress = false;
                m_aoapAttempts++;
                Logger::instance().warning(
                    QString("[RealAndroidAutoService] AOAP chain error (attempt %1): %2")
                        .arg(m_aoapAttempts)
                        .arg(QString::fromStdString(error.what())));
                if (m_aoapAttempts >= m_aoapMaxAttempts) {
                  Logger::instance().warning(QString("[RealAndroidAutoService] Reached %1 AOAP attempts, pausing retries for %2 ms")
                                               .arg(m_aoapMaxAttempts)
                                               .arg(m_aoapResetMs));
                  // Start/reset the retry reset timer
                  if (!m_aoapRetryResetTimer) {
                    m_aoapRetryResetTimer = new QTimer(this);
                    m_aoapRetryResetTimer->setSingleShot(true);
                    connect(m_aoapRetryResetTimer, &QTimer::timeout, this, [this]() {
                      Logger::instance().info("[RealAndroidAutoService] AOAP attempt window reset; allowing retries again");
                      m_aoapAttempts = 0;
                    });
                  }
                  m_aoapRetryResetTimer->start(m_aoapResetMs);
                }
                // Restart detection timer to retry
                if (m_deviceDetectionTimer && m_state == ConnectionState::SEARCHING) {
                  QTimer::singleShot(2000, this, [this]() {
                    if (m_deviceDetectionTimer && m_state == ConnectionState::SEARCHING) {
                      m_deviceDetectionTimer->start();
                    }
                  });
                }
              };
              
              aoapPromise->then(onSuccess, onError);
              
              Logger::instance().info("[RealAndroidAutoService] Starting AOAP query chain...");
              try {
                chain->start(std::move(handle), std::move(aoapPromise));
                Logger::instance().info("[RealAndroidAutoService] AOAP chain started successfully");
                
                // Set a timeout to check if device re-enumerated in AOAP mode
                // The chain may take longer on some devices; give it more time
                if (m_deviceDetectionTimer) {
                  QTimer::singleShot(8000, this, [this]() {
                    if (m_aoapInProgress) {
                      Logger::instance().info("[RealAndroidAutoService] AOAP timeout - checking if device re-enumerated...");
                      // This will trigger another device check which will look for AOAP mode
                      if (m_state == ConnectionState::SEARCHING) {
                        m_aoapInProgress = false;  // Reset flag to allow retry
                        m_deviceDetectionTimer->start();
                      }
                    }
                  });
                }
              } catch (const std::exception& e) {
                Logger::instance().error(QString("[RealAndroidAutoService] Exception starting AOAP chain: %1").arg(e.what()));
                m_aoapInProgress = false;
                if (m_deviceDetectionTimer && m_state == ConnectionState::SEARCHING) {
                  m_deviceDetectionTimer->start();
                }
              }
              
              // Stop the detection timer since we're attempting AOAP
              if (m_deviceDetectionTimer) {
                m_deviceDetectionTimer->stop();
              }
              // Increment attempt counter and guard against too many attempts
              m_aoapAttempts++;
              Logger::instance().debug(QString("[RealAndroidAutoService] AOAP attempt %1 started").arg(m_aoapAttempts));
            } else {
              Logger::instance().warning("[RealAndroidAutoService] Query chain factory or strand not available");
              m_aoapInProgress = false;
            }
          } else {
            Logger::instance().warning(QString("[RealAndroidAutoService] Failed to open device for AOAP (result=%1)").arg(openResult));
          }
        }
      }
    }
  } catch (const std::exception& e) {
    Logger::instance().debug(QString("[RealAndroidAutoService] Device check error: %1").arg(e.what()));
  }
}

void RealAndroidAutoService::onVideoFrame(const uint8_t* data, int size, int width, int height) {
  if (!isConnected()) {
    return;
  }

  emit videoFrameReady(width, height, data, size);
  updateStats();
}

void RealAndroidAutoService::onAudioData(const QByteArray& data) {
  if (!isConnected() || !m_audioEnabled) {
    return;
  }

  emit audioDataReady(data);
}

void RealAndroidAutoService::updateStats() {
  // TODO: Implement proper stats tracking
  emit statsUpdated(m_fps, m_latency, m_droppedFrames);
}

void RealAndroidAutoService::transitionToState(ConnectionState newState) {
  if (m_state == newState) {
    return;
  }

  m_state = newState;
  emit connectionStateChanged(newState);
}

void RealAndroidAutoService::onVideoChannelUpdate(const QByteArray& data, int width, int height) {
  if (!m_channelConfig.videoEnabled) {
    return;
  }

  // H.264 video data from Android device
  if (m_videoDecoder && m_videoDecoder->isReady()) {
    // Decode H.264 to RGBA using GStreamer
    aasdk::common::Data h264Data;
    h264Data.resize(data.size());
    std::copy(data.begin(), data.end(), h264Data.begin());

    // Convert aasdk::common::Data to QByteArray
    QByteArray frameData(reinterpret_cast<const char*>(h264Data.data()), h264Data.size());
    if (!m_videoDecoder->decodeFrame(frameData)) {
      Logger::instance().warning("Failed to decode video frame");
      m_droppedFrames++;
    }
  } else {
    // Fallback: emit raw H.264 data (for external decoder or testing)
    emit videoFrameReady(width, height, reinterpret_cast<const uint8_t*>(data.data()), data.size());
  }

  updateStats();
}

void RealAndroidAutoService::onMediaAudioChannelUpdate(const QByteArray& data) {
  if (!m_channelConfig.mediaAudioEnabled || !m_audioEnabled) {
    return;
  }

  // PCM audio data from Android device (music playback)
  if (m_audioMixer) {
    m_audioMixer->mixAudioData(IAudioMixer::ChannelId::MEDIA, data);
    Logger::instance().debug(QString("Media audio mixed: %1 bytes").arg(data.size()));
  } else {
    // Fallback: emit raw audio
    emit audioDataReady(data);
    Logger::instance().debug(QString("Media audio: %1 bytes").arg(data.size()));
  }
}

void RealAndroidAutoService::onSystemAudioChannelUpdate(const QByteArray& data) {
  if (!m_channelConfig.systemAudioEnabled || !m_audioEnabled) {
    return;
  }

  // PCM audio data from Android device (system sounds, notifications)
  if (m_audioMixer) {
    m_audioMixer->mixAudioData(IAudioMixer::ChannelId::SYSTEM, data);
    Logger::instance().debug(QString("System audio mixed: %1 bytes").arg(data.size()));
  } else {
    // Fallback: emit raw audio
    emit audioDataReady(data);
    Logger::instance().debug(QString("System audio: %1 bytes").arg(data.size()));
  }
}

void RealAndroidAutoService::onSpeechAudioChannelUpdate(const QByteArray& data) {
  if (!m_channelConfig.speechAudioEnabled || !m_audioEnabled) {
    return;
  }

  // PCM audio data from Android device (navigation guidance, voice assistant)
  if (m_audioMixer) {
    m_audioMixer->mixAudioData(IAudioMixer::ChannelId::SPEECH, data);
    Logger::instance().debug(QString("Speech audio mixed: %1 bytes").arg(data.size()));
  } else {
    // Fallback: emit raw audio
    emit audioDataReady(data);
    Logger::instance().debug(QString("Speech audio: %1 bytes").arg(data.size()));
  }
}

void RealAndroidAutoService::onSensorRequest() {
  if (!m_channelConfig.sensorEnabled) {
    return;
  }

  // Android device is requesting sensor data (GPS, speed, night mode, etc)
  // TODO: Implement sensor data collection and transmission
  Logger::instance().debug("Sensor data requested by Android device");
}

void RealAndroidAutoService::onBluetoothPairingRequest(const QString& deviceName) {
  if (!m_channelConfig.bluetoothEnabled) {
    return;
  }

  // Android device is requesting Bluetooth pairing
  Logger::instance().info(QString("Bluetooth pairing requested: %1").arg(deviceName));
  // TODO: Implement Bluetooth pairing flow
}

void RealAndroidAutoService::onChannelError(const QString& channelName, const QString& error) {
  Logger::instance().error(QString("Channel error [%1]: %2").arg(channelName, error));
  emit errorOccurred(QString("%1 channel error: %2").arg(channelName, error));
}
