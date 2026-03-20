// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — src/widgets/spinbox.cpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// Implementation of lv::SpinBox.

#include "lvgl/widgets/spinbox.hpp"

#include <algorithm>

namespace lv {

struct SpinBox::Impl {
  int32_t value_ = 0;
  int32_t min_ = 0;
  int32_t max_ = 999;
  int32_t step_ = 1;
  uint8_t digit_count_ = 3;
  uint8_t decimal_point_ = 0;
};

SpinBox::SpinBox(Object* parent)
    : Object(parent), impl_(std::make_unique<Impl>()) {
  add_flag(ObjFlags::Clickable);
}

SpinBox::~SpinBox() = default;

SpinBox& SpinBox::set_value(int32_t value) {
  impl_->value_ = std::clamp(value, impl_->min_, impl_->max_);
  invalidate();
  return *this;
}

SpinBox& SpinBox::set_range(int32_t min, int32_t max) {
  impl_->min_ = min;
  impl_->max_ = max;
  impl_->value_ = std::clamp(impl_->value_, min, max);
  invalidate();
  return *this;
}

SpinBox& SpinBox::set_step(int32_t step) {
  impl_->step_ = step;
  return *this;
}

SpinBox& SpinBox::set_digit_count(uint8_t count) {
  impl_->digit_count_ = count;
  invalidate();
  return *this;
}

SpinBox& SpinBox::set_decimal_point(uint8_t pos) {
  impl_->decimal_point_ = pos;
  invalidate();
  return *this;
}

SpinBox& SpinBox::increment() {
  set_value(impl_->value_ + impl_->step_);
  return *this;
}

SpinBox& SpinBox::decrement() {
  set_value(impl_->value_ - impl_->step_);
  return *this;
}

int32_t SpinBox::value() const noexcept {
  return impl_->value_;
}

int32_t SpinBox::step() const noexcept {
  return impl_->step_;
}

}  // namespace lv
