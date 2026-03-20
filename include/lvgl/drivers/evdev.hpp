// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — drivers/evdev.hpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// Linux evdev input drivers for pointer (touchscreen/mouse) and keypad.

#pragma once

#include <cstdint>
#include <memory>
#include <string_view>

#include "input_device.hpp"

namespace lv::drivers {

// ── EvdevPointer ────────────────────────────────────────────────────────────
class EvdevPointer : public lv::Pointer {
 public:
  explicit EvdevPointer(std::string_view device = "/dev/input/event0");
  ~EvdevPointer() override;

  EvdevPointer(const EvdevPointer&) = delete;
  EvdevPointer& operator=(const EvdevPointer&) = delete;

  // Set screen dimensions for coordinate mapping
  void set_calibration(int32_t screen_w, int32_t screen_h) noexcept;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

// ── EvdevKeypad ─────────────────────────────────────────────────────────────
class EvdevKeypad : public lv::Keypad {
 public:
  explicit EvdevKeypad(std::string_view device = "/dev/input/event1");
  ~EvdevKeypad() override;

  EvdevKeypad(const EvdevKeypad&) = delete;
  EvdevKeypad& operator=(const EvdevKeypad&) = delete;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace lv::drivers
