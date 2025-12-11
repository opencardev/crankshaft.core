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

#include "ProtocolHelpers.h"

#include <chrono>

namespace crankshaft {
namespace protocol {

uint64_t getCurrentTimestampMicros() {
  auto now = std::chrono::system_clock::now();
  auto duration = now.time_since_epoch();
  return std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
}

aap_protobuf::service::inputsource::message::InputReport createTouchInputReport(
    float x, float y, TouchAction action, int pointerId, uint64_t timestamp) {
  // Create InputReport message
  aap_protobuf::service::inputsource::message::InputReport inputReport;

  // Set timestamp (microseconds)
  if (timestamp == 0) {
    timestamp = getCurrentTimestampMicros();
  }
  inputReport.set_timestamp(timestamp);

  // Create TouchEvent
  auto* touchEvent = inputReport.mutable_touch_event();

  // Add pointer data
  auto* pointerData = touchEvent->add_pointer_data();
  pointerData->set_x(x);
  pointerData->set_y(y);
  pointerData->set_pointer_id(pointerId);

  // Set touch action
  switch (action) {
    case TouchAction::ACTION_DOWN:
      touchEvent->set_action(
          aap_protobuf::service::inputsource::message::PointerAction::ACTION_DOWN);
      break;
    case TouchAction::ACTION_UP:
      touchEvent->set_action(aap_protobuf::service::inputsource::message::PointerAction::ACTION_UP);
      break;
    case TouchAction::ACTION_MOVED:
      touchEvent->set_action(
          aap_protobuf::service::inputsource::message::PointerAction::ACTION_MOVED);
      break;
  }

  // Set action index (pointer index that triggered the action)
  touchEvent->set_action_index(0);

  return inputReport;
}

aap_protobuf::service::inputsource::message::InputReport createKeyInputReport(
    int keyCode, KeyAction action, uint64_t timestamp, bool longPress, int metaState) {
  // Create InputReport message
  aap_protobuf::service::inputsource::message::InputReport inputReport;

  // Set timestamp (microseconds)
  if (timestamp == 0) {
    timestamp = getCurrentTimestampMicros();
  }
  inputReport.set_timestamp(timestamp);

  // Create KeyEvent
  auto* keyEvent = inputReport.mutable_key_event();

  // Add key
  auto* key = keyEvent->add_keys();
  key->set_keycode(keyCode);
  key->set_down(action == KeyAction::ACTION_DOWN);
  key->set_longpress(longPress);
  key->set_metastate(metaState);

  return inputReport;
}

aap_protobuf::service::control::message::AudioFocusNotification createAudioFocusNotification(
    AudioFocusState focusState) {
  // Create AudioFocusNotification message
  aap_protobuf::service::control::message::AudioFocusNotification notification;

  // Set focus state
  switch (focusState) {
    case AudioFocusState::GAIN:
      notification.set_focus_state(aap_protobuf::service::control::message::AUDIO_FOCUS_STATE_GAIN);
      break;
    case AudioFocusState::LOSS:
      notification.set_focus_state(aap_protobuf::service::control::message::AUDIO_FOCUS_STATE_LOSS);
      break;
  }

  return notification;
}

}  // namespace protocol
}  // namespace crankshaft
