// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — src/widgets/slider.cpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// Implementation of lv::Slider.

#include "lvgl/widgets/slider.hpp"

#include <algorithm>

namespace lv {

struct Slider::Impl {
  int32_t value_ = 0;
  int32_t left_value_ = 0;
  int32_t min_ = 0;
  int32_t max_ = 100;
  SliderMode mode_ = SliderMode::Normal;
  bool dragged_ = false;
};

Slider::Slider(Object* parent)
    : Object(parent), impl_(std::make_unique<Impl>()) {
  add_flag(ObjFlags::Clickable);
}

Slider::~Slider() = default;

Slider& Slider::set_value(int32_t value, AnimEnable /*anim*/) {
  impl_->value_ = std::clamp(value, impl_->min_, impl_->max_);
  invalidate();
  return *this;
}

Slider& Slider::set_left_value(int32_t value, AnimEnable /*anim*/) {
  impl_->left_value_ = std::clamp(value, impl_->min_, impl_->max_);
  invalidate();
  return *this;
}

Slider& Slider::set_range(int32_t min, int32_t max) {
  impl_->min_ = min;
  impl_->max_ = max;
  impl_->value_ = std::clamp(impl_->value_, min, max);
  impl_->left_value_ = std::clamp(impl_->left_value_, min, max);
  invalidate();
  return *this;
}

Slider& Slider::set_mode(SliderMode mode) {
  impl_->mode_ = mode;
  invalidate();
  return *this;
}

int32_t Slider::value() const noexcept {
  return impl_->value_;
}

int32_t Slider::left_value() const noexcept {
  return impl_->left_value_;
}

int32_t Slider::min_value() const noexcept {
  return impl_->min_;
}

int32_t Slider::max_value() const noexcept {
  return impl_->max_;
}

SliderMode Slider::mode() const noexcept {
  return impl_->mode_;
}

bool Slider::is_dragged() const noexcept {
  return impl_->dragged_;
}

}  // namespace lv
