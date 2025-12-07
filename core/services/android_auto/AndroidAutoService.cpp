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
#include "../logging/Logger.h"
#include <QTimer>
#include <QDebug>
#include <libusb-1.0/libusb.h>
#include <QJsonDocument>

class AndroidAutoServiceImpl : public AndroidAutoService {
  Q_OBJECT

 public:
  explicit AndroidAutoServiceImpl(MediaPipeline* mediaPipeline, QObject* parent = nullptr)
      : AndroidAutoService(parent),
        state_(ConnectionState::DISCONNECTED),
        search_timer_(nullptr),
        media_pipeline_(mediaPipeline),
        usb_context_(nullptr),
        display_width_(1280),
        display_height_(720),
        framerate_(30),
        audio_enabled_(true),
        frame_drop_count_(0),
        latency_ms_(0) {}

  ~AndroidAutoServiceImpl() override { deinitialise(); }

  bool initialise() override {
    Logger::instance().info(QString("[AndroidAuto] Initialising Android Auto service"));

    // Initialize libusb
    int ret = libusb_init(&usb_context_);
    if (ret < 0) {
      Logger::instance().error(QString("[AndroidAuto] Failed to initialize libusb: %1").arg(libusb_error_name(ret)));
      return false;
    }

    Logger::instance().info(QString("[AndroidAuto] libusb initialized successfully"));
    return true;
  }

  void deinitialise() override {
    Logger::instance().info(QString("[AndroidAuto] Deinitialising Android Auto service"));
    stopSearching();

    if (usb_context_) {
      libusb_exit(usb_context_);
      usb_context_ = nullptr;
    }
  }

  ConnectionState getConnectionState() const override { return state_; }

  bool isConnected() const override { return state_ == ConnectionState::CONNECTED; }

  AndroidDevice getConnectedDevice() const override { return connected_device_; }

  bool startSearching() override {
    if (state_ != ConnectionState::DISCONNECTED) {
      return false;
    }

    Logger::instance().info(QString("[AndroidAuto] Starting device search"));
    setState(ConnectionState::SEARCHING);

    if (!usb_context_) {
      Logger::instance().warning(QString("[AndroidAuto] libusb not initialized"));
      return false;
    }

    if (!search_timer_) {
      search_timer_ = new QTimer(this);
      connect(search_timer_, &QTimer::timeout, this, &AndroidAutoServiceImpl::onSearchTimeout);
    }

    enumerateUSBDevices();
    search_timer_->start(2000);

    return true;
  }

  void stopSearching() override {
    Logger::instance().info(QString("[AndroidAuto] Stopping device search"));
    if (search_timer_) {
      search_timer_->stop();
    }
    if (state_ == ConnectionState::SEARCHING) {
      setState(ConnectionState::DISCONNECTED);
    }
  }

  bool connectToDevice(const QString& serial) override {
    if (state_ != ConnectionState::DISCONNECTED && state_ != ConnectionState::SEARCHING) {
      Logger::instance().warning(QString("[AndroidAuto] Invalid state for connection: %1").arg(static_cast<int>(state_)));
      return false;
    }

    Logger::instance().info(QString("[AndroidAuto] Attempting to connect to device: %1").arg(serial));
    setState(ConnectionState::CONNECTING);

    // In a real implementation, this would set up USB enumeration filters,
    // negotiate accessory mode, and establish messenger channels
    // For now, simulate successful connection
    connected_device_.serialNumber = serial;
    connected_device_.manufacturer = "Android";
    connected_device_.model = "Virtual";
    connected_device_.androidVersion = "11.0";
    connected_device_.connected = true;

    setState(ConnectionState::CONNECTED);
    emit connected(connected_device_);

    return true;
  }

  bool disconnect() override {
    if (!isConnected()) {
      return false;
    }

    Logger::instance().info(QString("[AndroidAuto] Disconnecting from device"));
    setState(ConnectionState::DISCONNECTING);

    connected_device_ = {};
    setState(ConnectionState::DISCONNECTED);
    emit disconnected();

    return true;
  }

  bool setDisplayResolution(const QSize& resolution) override {
    display_width_ = resolution.width();
    display_height_ = resolution.height();
    Logger::instance().info(QString("[AndroidAuto] Display resolution set to %1x%2").arg(display_width_).arg(display_height_));
    return true;
  }

  QSize getDisplayResolution() const override {
    return QSize(display_width_, display_height_);
  }

  bool setFramerate(int fps) override {
    framerate_ = fps;
    Logger::instance().info(QString("[AndroidAuto] Framerate set to %1 fps").arg(fps));
    return true;
  }

  int getFramerate() const override { return framerate_; }

  bool sendTouchInput(int x, int y, int action) override {
    if (!isConnected()) {
      return false;
    }
    Logger::instance().debug(QString("[AndroidAuto] Touch input: (%1, %2) action=%3").arg(x).arg(y).arg(action));
    return true;
  }

  bool sendKeyInput(int key_code, int action) override {
    if (!isConnected()) {
      return false;
    }
    Logger::instance().debug(QString("[AndroidAuto] Key input: code=%1 action=%2").arg(key_code).arg(action));
    return true;
  }

  bool requestAudioFocus() override {
    if (!isConnected()) {
      return false;
    }
    Logger::instance().info(QString("[AndroidAuto] Requesting audio focus"));
    return true;
  }

  bool abandonAudioFocus() override {
    if (!isConnected()) {
      return false;
    }
    Logger::instance().info(QString("[AndroidAuto] Abandoning audio focus"));
    return true;
  }

  int getFrameDropCount() const override { return frame_drop_count_; }

  int getLatency() const override { return latency_ms_; }

  bool setAudioEnabled(bool enabled) override {
    audio_enabled_ = enabled;
    Logger::instance().info(QString("[AndroidAuto] Audio %1").arg(enabled ? "enabled" : "disabled"));
    return true;
  }

  QJsonObject getAudioConfig() const override {
    QJsonObject config;
    config["enabled"] = audio_enabled_;
    config["sampleRate"] = 48000;
    config["channels"] = 2;
    config["bitDepth"] = 16;
    return config;
  }

 protected:
  void setState(ConnectionState newState) {
    if (state_ != newState) {
      state_ = newState;
      emit connectionStateChanged(state_);
    }
  }

 private slots:
  void onSearchTimeout() {
    // Periodic USB device enumeration
    enumerateUSBDevices();
  }

 private:
  void enumerateUSBDevices() {
    if (!usb_context_) {
      return;
    }

    libusb_device** devices;
    ssize_t device_count = libusb_get_device_list(usb_context_, &devices);

    if (device_count < 0) {
      Logger::instance().error(QString("[AndroidAuto] Failed to enumerate USB devices"));
      return;
    }

    Logger::instance().debug(QString("[AndroidAuto] Found %1 USB devices").arg(device_count));

    libusb_free_device_list(devices, 1);
  }

  ConnectionState state_;
  AndroidDevice connected_device_;
  QTimer* search_timer_;
  MediaPipeline* media_pipeline_;
  libusb_context* usb_context_;
  int display_width_;
  int display_height_;
  int framerate_;
  bool audio_enabled_;
  int frame_drop_count_;
  int latency_ms_;
};

// Base class constructor
AndroidAutoService::AndroidAutoService(QObject* parent) : QObject(parent) {}

// Base class destructor
AndroidAutoService::~AndroidAutoService() {}

// Static factory function
AndroidAutoService* AndroidAutoService::create(MediaPipeline* mediaPipeline, QObject* parent) {
  return new AndroidAutoServiceImpl(mediaPipeline, parent);
}

#include "AndroidAutoService.moc"
