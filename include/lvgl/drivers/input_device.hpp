// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — drivers/input_device.hpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// Input device abstractions: Pointer, Keypad, Encoder.

#pragma once

#include <cstdint>
#include <functional>
#include <memory>

#include "concepts.hpp"

namespace lv {

// Forward declarations
class Display;
class Group;

// ── InputDevice base ────────────────────────────────────────────────────────
class InputDevice {
 public:
  using ReadFn = std::function<void(drivers::InputState&)>;

  virtual ~InputDevice() = default;

  InputDevice& set_read(ReadFn fn) {
    read_fn_ = std::move(fn);
    return *this;
  }

  InputDevice& set_display(Display& disp) {
    display_ = &disp;
    return *this;
  }

  InputDevice& set_group(Group& group) {
    group_ = &group;
    return *this;
  }

  void poll() {
    if (read_fn_) {
      read_fn_(state_);
    }
  }

  [[nodiscard]] const drivers::InputState& state() const noexcept {
    return state_;
  }
  [[nodiscard]] Display* display() const noexcept { return display_; }
  [[nodiscard]] Group* group() const noexcept { return group_; }

 protected:
  InputDevice() = default;

  ReadFn read_fn_;
  drivers::InputState state_;
  Display* display_ = nullptr;
  Group* group_ = nullptr;
};

// ── Pointer (mouse/touch) ───────────────────────────────────────────────────
class Pointer : public InputDevice {
 public:
  Pointer() { state_.type = drivers::InputState::Type::Pointer; }

  [[nodiscard]] int32_t x() const noexcept { return state_.x; }
  [[nodiscard]] int32_t y() const noexcept { return state_.y; }
  [[nodiscard]] bool pressed() const noexcept { return state_.pressed; }
};

// ── Keypad ──────────────────────────────────────────────────────────────────
class Keypad : public InputDevice {
 public:
  Keypad() { state_.type = drivers::InputState::Type::Keypad; }

  [[nodiscard]] uint32_t key() const noexcept { return state_.key; }
  [[nodiscard]] bool pressed() const noexcept { return state_.pressed; }
};

// ── Encoder ─────────────────────────────────────────────────────────────────
class Encoder : public InputDevice {
 public:
  Encoder() { state_.type = drivers::InputState::Type::Encoder; }

  [[nodiscard]] int32_t diff() const noexcept { return state_.encoder_diff; }
  [[nodiscard]] bool pressed() const noexcept { return state_.pressed; }
};

}  // namespace lv
