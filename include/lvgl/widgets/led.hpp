// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — widgets/led.hpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// lv::Led — LED indicator widget with brightness control.

#pragma once

#include <memory>

#include "../core/object.hpp"
#include "../core/types.hpp"
#include "../misc/color.hpp"

namespace lv {

class Led : public Object {
 public:
  explicit Led(Object* parent);
  ~Led() override;

  Led& set_brightness(uint8_t brightness);
  Led& set_color(Color color);
  Led& on();
  Led& off();
  Led& toggle();

  [[nodiscard]] uint8_t brightness() const noexcept;
  [[nodiscard]] Color color() const noexcept;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace lv
