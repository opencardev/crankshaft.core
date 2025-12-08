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

#ifndef PROTOCOLHELPERS_H
#define PROTOCOLHELPERS_H

#include <aasdk/Common/Data.hpp>
#include <aap_protobuf/service/control/message/AudioFocusNotification.pb.h>
#include <aap_protobuf/service/inputsource/message/InputReport.pb.h>
#include <aap_protobuf/service/inputsource/message/TouchEvent.pb.h>
#include <aap_protobuf/service/inputsource/message/KeyEvent.pb.h>

namespace crankshaft {
namespace protocol {

/**
 * @brief Touch action types for touch input events
 */
enum class TouchAction {
  ACTION_DOWN = 0,   // Finger touches screen
  ACTION_UP = 1,     // Finger lifts from screen
  ACTION_MOVED = 2   // Finger moves on screen
};

/**
 * @brief Key action types for key input events
 */
enum class KeyAction {
  ACTION_DOWN = 0,   // Key pressed
  ACTION_UP = 1      // Key released
};

/**
 * @brief Audio focus state types
 */
enum class AudioFocusState {
  GAIN = 1,   // Give audio focus to Android Auto
  LOSS = 2    // Take audio focus from Android Auto
};

/**
 * @brief Create a touch input report message
 * @param x X coordinate (0-1 range, will be scaled to display width)
 * @param y Y coordinate (0-1 range, will be scaled to display height)
 * @param action Touch action (DOWN, UP, MOVED)
 * @param pointerId Unique pointer identifier for multi-touch
 * @param timestamp Event timestamp in microseconds
 * @return InputReport protobuf message
 */
aap_protobuf::service::inputsource::message::InputReport createTouchInputReport(float x, float y, TouchAction action,
                                          int pointerId = 0,
                                          uint64_t timestamp = 0);

/**
 * @brief Create a key input report message
 * @param keyCode Android KeyEvent keycode
 * @param action Key action (DOWN, UP)
 * @param timestamp Event timestamp in microseconds
 * @param longPress Whether this is a long press event
 * @param metaState Key modifier state (Shift, Ctrl, Alt, etc.)
 * @return InputReport protobuf message
 */
aap_protobuf::service::inputsource::message::InputReport createKeyInputReport(int keyCode, KeyAction action,
                                        uint64_t timestamp = 0,
                                        bool longPress = false,
                                        int metaState = 0);

/**
 * @brief Create an audio focus notification message
 * @param focusState Focus state to set (GAIN or LOSS)
 * @return AudioFocusNotification protobuf message
 */
aap_protobuf::service::control::message::AudioFocusNotification createAudioFocusNotification(AudioFocusState focusState);

/**
 * @brief Get current timestamp in microseconds
 * @return Timestamp suitable for input events
 */
uint64_t getCurrentTimestampMicros();

}  // namespace protocol
}  // namespace crankshaft

#endif  // PROTOCOLHELPERS_H
