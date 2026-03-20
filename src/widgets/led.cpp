// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — src/widgets/led.cpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// Implementation of lv::Led.

#include "lvgl/widgets/led.hpp"

namespace lv {

struct Led::Impl {
  uint8_t brightness_ = 128;
  Color color_ = Color::Green();
};

Led::Led(Object* parent) : Object(parent), impl_(std::make_unique<Impl>()) {}

Led::~Led() = default;

Led& Led::set_brightness(uint8_t brightness) {
  impl_->brightness_ = brightness;
  invalidate();
  return *this;
}

Led& Led::set_color(Color color) {
  impl_->color_ = color;
  invalidate();
  return *this;
}

Led& Led::on() {
  impl_->brightness_ = 255;
  invalidate();
  return *this;
}

Led& Led::off() {
  impl_->brightness_ = 0;
  invalidate();
  return *this;
}

Led& Led::toggle() {
  impl_->brightness_ = (impl_->brightness_ == 0) ? uint8_t{255} : uint8_t{0};
  invalidate();
  return *this;
}

uint8_t Led::brightness() const noexcept {
  return impl_->brightness_;
}

Color Led::color() const noexcept {
  return impl_->color_;
}

}  // namespace lv
