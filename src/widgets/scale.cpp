// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — src/widgets/scale.cpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// Implementation of lv::Scale.

#include "lvgl/widgets/scale.hpp"

namespace lv {

struct Scale::Impl {
  ScaleMode mode_ = ScaleMode::HorizontalBottom;
  int32_t min_ = 0;
  int32_t max_ = 100;
  int32_t total_tick_count_ = 11;
  int32_t major_tick_every_ = 5;
  int32_t label_show_ = 1;
};

Scale::Scale(Object* parent)
    : Object(parent), impl_(std::make_unique<Impl>()) {}

Scale::~Scale() = default;

Scale& Scale::set_mode(ScaleMode mode) {
  impl_->mode_ = mode;
  invalidate();
  return *this;
}

Scale& Scale::set_range(int32_t min, int32_t max) {
  impl_->min_ = min;
  impl_->max_ = max;
  invalidate();
  return *this;
}

Scale& Scale::set_total_tick_count(int32_t count) {
  impl_->total_tick_count_ = count;
  invalidate();
  return *this;
}

Scale& Scale::set_major_tick_every(int32_t nth) {
  impl_->major_tick_every_ = nth;
  invalidate();
  return *this;
}

Scale& Scale::set_label_show(bool show) {
  impl_->label_show_ = show ? 1 : 0;
  invalidate();
  return *this;
}

ScaleMode Scale::mode() const noexcept {
  return impl_->mode_;
}

int32_t Scale::min_value() const noexcept {
  return impl_->min_;
}

int32_t Scale::max_value() const noexcept {
  return impl_->max_;
}

}  // namespace lv
