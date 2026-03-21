// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — src/widgets/arclabel.cpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// Implementation of lv::ArcLabel.

#include "lvgl/widgets/arclabel.hpp"

namespace lv {

struct ArcLabel::Impl {
  std::string text_;
  int32_t radius_ = 100;
  int32_t start_angle_ = 0;
};

ArcLabel::ArcLabel(Object* parent)
    : Object(parent), impl_(std::make_unique<Impl>()) {}

ArcLabel::~ArcLabel() = default;

ArcLabel& ArcLabel::set_text(std::string_view text) {
  impl_->text_ = text;
  invalidate();
  return *this;
}

ArcLabel& ArcLabel::set_radius(int32_t radius) {
  impl_->radius_ = radius;
  invalidate();
  return *this;
}

ArcLabel& ArcLabel::set_start_angle(int32_t angle) {
  impl_->start_angle_ = angle;
  invalidate();
  return *this;
}

std::string_view ArcLabel::text() const noexcept {
  return impl_->text_;
}

int32_t ArcLabel::radius() const noexcept {
  return impl_->radius_;
}

int32_t ArcLabel::start_angle() const noexcept {
  return impl_->start_angle_;
}

}  // namespace lv
