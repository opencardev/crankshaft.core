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

#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <cstring>
#include <cstdlib>

#include <libusb.h>
#include <boost/asio.hpp>
#include <aasdk/USB/USBHub.hpp>
#include <aasdk/USB/USBWrapper.hpp>
#include <aasdk/USB/AccessoryModeQueryFactory.hpp>
#include <aasdk/USB/AccessoryModeQueryChainFactory.hpp>
#include <aasdk/Common/ModernLogger.hpp>
#include <aasdk/IO/Promise.hpp>
#include <aasdk/Error/Error.hpp>

namespace asio = boost::asio;
using DeviceHandle = std::shared_ptr<libusb_device_handle>;
using PromiseType = aasdk::io::Promise<DeviceHandle>;

class AATest {
public:
    AATest()
        : ioService_()
        , work_(asio::make_work_guard(ioService_))
        , usbContext_(nullptr)
        , usbWrapper_(nullptr)
        , queryFactory_(nullptr)
        , queryChainFactory_(nullptr)
    {
        // Initialize libusb context
        int result = libusb_init(&usbContext_);
        if (result != 0) {
            throw std::runtime_error(std::string("Failed to initialize libusb: ") + libusb_error_name(result));
        }

        // Create USB wrapper with the context
        usbWrapper_ = std::make_unique<aasdk::usb::USBWrapper>(usbContext_);

        // Create factories
        queryFactory_ = std::make_unique<aasdk::usb::AccessoryModeQueryFactory>(*usbWrapper_, ioService_);
        queryChainFactory_ = std::make_unique<aasdk::usb::AccessoryModeQueryChainFactory>(*usbWrapper_, ioService_, *queryFactory_);
    }

    ~AATest() {
        if (usbContext_) {
            libusb_exit(usbContext_);
        }
    }

    void run() {
        std::cout << "[AATest] Starting Android Auto AOAP negotiation test tool..." << std::endl;
        
        // Start io_service in background thread
        ioThread_ = std::thread([this]() {
            ioService_.run();
        });

        // Keep running and handle libusb events
        int pollCount = 0;
        while (true) {
            try {
                usbWrapper_->handleEvents();
                
                // Enumerate devices periodically (every 10 poll cycles)
                if (++pollCount >= 10) {
                    enumerateAndConnect();
                    pollCount = 0;
                }
            } catch (const std::exception& e) {
                std::cerr << "[AATest] USB event error: " << e.what() << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    void stop() {
        work_.reset();
        ioService_.stop();
        if (ioThread_.joinable()) {
            ioThread_.join();
        }
    }

private:
    void enumerateAndConnect() {
        std::cout << "[AATest] Enumerating USB devices..." << std::endl;
        
        libusb_device** devices;
        ssize_t count = libusb_get_device_list(usbContext_, &devices);
        
        if (count < 0) {
            std::cerr << "[AATest] Failed to get device list" << std::endl;
            return;
        }

        bool found = false;
        for (ssize_t i = 0; i < count; ++i) {
            libusb_device_descriptor desc;
            if (libusb_get_device_descriptor(devices[i], &desc) == 0) {
                // Google vendor ID
                if (desc.idVendor == 0x18d1) {
                    std::cout << "[AATest] Found Google device: vid=0x" << std::hex << desc.idVendor 
                              << " pid=0x" << desc.idProduct << std::dec << std::endl;
                    
                    // Skip if already in accessory mode
                    if (desc.idProduct == 0x2d00 || desc.idProduct == 0x2d01) {
                        std::cout << "[AATest] Device already in accessory mode" << std::endl;
                        found = true;
                        continue;
                    }
                    
                    // Attempt AOAP negotiation on this device
                    if (!found) {
                        attemptAOAP(devices[i]);
                        found = true;
                    }
                }
            }
        }

        libusb_free_device_list(devices, 1);

        if (!found) {
            std::cout << "[AATest] No Google devices found. Plug in your Android device." << std::endl;
        }
    }

    void attemptAOAP(libusb_device* device) {
        std::cout << "[AATest] Attempting AOAP negotiation..." << std::endl;

        libusb_device_handle* rawHandle = nullptr;
        int result = libusb_open(device, &rawHandle);
        if (result != 0) {
            std::cerr << "[AATest] Failed to open device: " << libusb_error_name(result) << std::endl;
            return;
        }

        std::cout << "[AATest] Device opened successfully" << std::endl;

        try {
            // Wrap the raw handle in a shared_ptr with a custom deleter
            DeviceHandle handle(rawHandle, [](libusb_device_handle* h) {
                if (h) libusb_close(h);
            });

            // Create AOAP query chain
            auto queryChain = queryChainFactory_->create();
            
            std::cout << "[AATest] Starting AOAP query chain..." << std::endl;

            // Create a promise for the chain result (needs io_service)
            auto promise = std::make_shared<PromiseType>(ioService_);

            // Start the chain - this will execute GET_PROTOCOL, SEND_STRING (x6), START
            queryChain->start(
                handle,
                promise
            );

            // Set up callbacks to handle completion
            promise->then(
                [this](const DeviceHandle& resultHandle) {
                    std::cout << "[AATest] AOAP chain completed successfully!" << std::endl;
                    std::cout << "[AATest] Device should now re-enumerate as accessory (18d1:2d00 or 18d1:2d01)" << std::endl;
                },
                [](const aasdk::error::Error& error) {
                    std::cerr << "[AATest] AOAP chain failed: " << error.message() << std::endl;
                }
            );

            std::cout << "[AATest] AOAP chain started, waiting for completion..." << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "[AATest] Exception during AOAP: " << e.what() << std::endl;
        }
    }

    asio::io_service ioService_;
    asio::executor_work_guard<asio::io_service::executor_type> work_;
    std::thread ioThread_;
    libusb_context* usbContext_;
    std::unique_ptr<aasdk::usb::USBWrapper> usbWrapper_;
    std::unique_ptr<aasdk::usb::AccessoryModeQueryFactory> queryFactory_;
    std::unique_ptr<aasdk::usb::AccessoryModeQueryChainFactory> queryChainFactory_;
};

void printUsage(const char* progName) {
    std::cout << "Usage: " << progName << " [options]\n"
              << "Options:\n"
              << "  --verbose-usb, -v    Enable verbose USB/AOAP logging\n"
              << "  --help, -h           Show this help\n"
              << "\n"
              << "Environment variables:\n"
              << "  AASDK_VERBOSE_USB=1  Enable verbose USB/AOAP logging\n"
              << std::endl;
}

int main(int argc, char* argv[]) {
    std::cout << "=== Crankshaft Android Auto AOAP Test Tool ===" << std::endl;

    // Check for verbose USB flag
    bool verboseUsb = false;
    const char* envVar = std::getenv("AASDK_VERBOSE_USB");
    if (envVar && (strcmp(envVar, "1") == 0 || strcmp(envVar, "true") == 0)) {
        verboseUsb = true;
    }

    // Parse command line
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--verbose-usb") == 0 || strcmp(argv[i], "-v") == 0) {
            verboseUsb = true;
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printUsage(argv[0]);
            return 0;
        }
    }

    // Enable verbose USB logging in AASDK if requested
    if (verboseUsb) {
        try {
            aasdk::common::ModernLogger::getInstance().setVerboseUsb(true);
            std::cout << "[AATest] Verbose USB/AOAP logging enabled" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "[AATest] Warning: Could not enable verbose USB logging: " << e.what() << std::endl;
        }
    } else {
        std::cout << "[AATest] Verbose USB logging disabled. Use --verbose-usb or AASDK_VERBOSE_USB=1 to enable." << std::endl;
    }

    try {
        AATest test;
        test.run();
    } catch (const std::exception& e) {
        std::cerr << "[AATest] Fatal error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
