// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — src/widgets/bar.cpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// Implementation of lv::Bar.

#include "lvgl/widgets/bar.hpp"

#include <algorithm>

namespace lv {

struct Bar::Impl {
  int32_t value_ = 0;
  int32_t start_value_ = 0;
  int32_t min_ = 0;
  int32_t max_ = 100;
  BarMode mode_ = BarMode::Normal;
};

Bar::Bar(Object* parent) : Object(parent), impl_(std::make_unique<Impl>()) {}

Bar::~Bar() = default;

Bar& Bar::set_value(int32_t value, AnimEnable /*anim*/) {
  impl_->value_ = std::clamp(value, impl_->min_, impl_->max_);
  invalidate();
  return *this;
}

Bar& Bar::set_start_value(int32_t value, AnimEnable /*anim*/) {
  impl_->start_value_ = std::clamp(value, impl_->min_, impl_->max_);
  invalidate();
  return *this;
}

Bar& Bar::set_range(int32_t min, int32_t max) {
  impl_->min_ = min;
  impl_->max_ = max;
  impl_->value_ = std::clamp(impl_->value_, min, max);
  impl_->start_value_ = std::clamp(impl_->start_value_, min, max);
  invalidate();
  return *this;
}

Bar& Bar::set_mode(BarMode mode) {
  impl_->mode_ = mode;
  invalidate();
  return *this;
}

int32_t Bar::value() const noexcept {
  return impl_->value_;
}

int32_t Bar::start_value() const noexcept {
  return impl_->start_value_;
}

int32_t Bar::min_value() const noexcept {
  return impl_->min_;
}

int32_t Bar::max_value() const noexcept {
  return impl_->max_;
}

}  // namespace lv
