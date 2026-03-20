// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — src/widgets/arc.cpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// Implementation of lv::Arc.

#include "lvgl/widgets/arc.hpp"

#include <algorithm>

namespace lv {

struct Arc::Impl {
  int32_t start_angle_ = 0;
  int32_t end_angle_ = 360;
  int32_t bg_start_angle_ = 0;
  int32_t bg_end_angle_ = 360;
  int32_t value_ = 0;
  int32_t min_ = 0;
  int32_t max_ = 100;
  int32_t rotation_ = 0;
  ArcMode mode_ = ArcMode::Normal;
};

Arc::Arc(Object* parent) : Object(parent), impl_(std::make_unique<Impl>()) {}

Arc::~Arc() = default;

Arc& Arc::set_start_angle(int32_t angle) {
  impl_->start_angle_ = angle;
  invalidate();
  return *this;
}

Arc& Arc::set_end_angle(int32_t angle) {
  impl_->end_angle_ = angle;
  invalidate();
  return *this;
}

Arc& Arc::set_bg_start_angle(int32_t angle) {
  impl_->bg_start_angle_ = angle;
  invalidate();
  return *this;
}

Arc& Arc::set_bg_end_angle(int32_t angle) {
  impl_->bg_end_angle_ = angle;
  invalidate();
  return *this;
}

Arc& Arc::set_value(int32_t value) {
  impl_->value_ = std::clamp(value, impl_->min_, impl_->max_);
  invalidate();
  return *this;
}

Arc& Arc::set_range(int32_t min, int32_t max) {
  impl_->min_ = min;
  impl_->max_ = max;
  impl_->value_ = std::clamp(impl_->value_, min, max);
  invalidate();
  return *this;
}

Arc& Arc::set_mode(ArcMode mode) {
  impl_->mode_ = mode;
  invalidate();
  return *this;
}

Arc& Arc::set_rotation(int32_t rotation) {
  impl_->rotation_ = rotation;
  invalidate();
  return *this;
}

int32_t Arc::angle_start() const noexcept {
  return impl_->start_angle_;
}

int32_t Arc::angle_end() const noexcept {
  return impl_->end_angle_;
}

int32_t Arc::value() const noexcept {
  return impl_->value_;
}

int32_t Arc::min_value() const noexcept {
  return impl_->min_;
}

int32_t Arc::max_value() const noexcept {
  return impl_->max_;
}

int32_t Arc::bg_angle_start() const noexcept {
  return impl_->bg_start_angle_;
}

int32_t Arc::bg_angle_end() const noexcept {
  return impl_->bg_end_angle_;
}

int32_t Arc::rotation() const noexcept {
  return impl_->rotation_;
}

}  // namespace lv
