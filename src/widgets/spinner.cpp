// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — src/widgets/spinner.cpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// Implementation of lv::Spinner.

#include "lvgl/widgets/spinner.hpp"

namespace lv {

struct Spinner::Impl {
  uint32_t time_ms_ = 1000;
  int32_t arc_length_ = 60;
};

Spinner::Spinner(Object* parent)
    : Object(parent), impl_(std::make_unique<Impl>()) {}

Spinner::~Spinner() = default;

Spinner& Spinner::set_anim_params(uint32_t time_ms, int32_t arc_length) {
  impl_->time_ms_ = time_ms;
  impl_->arc_length_ = arc_length;
  invalidate();
  return *this;
}

uint32_t Spinner::anim_time() const noexcept {
  return impl_->time_ms_;
}

int32_t Spinner::arc_length() const noexcept {
  return impl_->arc_length_;
}

}  // namespace lv
