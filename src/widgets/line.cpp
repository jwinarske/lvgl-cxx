// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — src/widgets/line.cpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// Implementation of lv::Line.

#include "lvgl/widgets/line.hpp"

#include <vector>

namespace lv {

struct Line::Impl {
  std::vector<Point> points_;
  bool y_invert_ = false;
};

Line::Line(Object* parent) : Object(parent), impl_(std::make_unique<Impl>()) {}

Line::~Line() = default;

Line& Line::set_points(std::span<const Point> pts) {
  impl_->points_.assign(pts.begin(), pts.end());
  invalidate();
  return *this;
}

Line& Line::set_y_invert(bool inv) {
  impl_->y_invert_ = inv;
  invalidate();
  return *this;
}

bool Line::y_invert() const noexcept {
  return impl_->y_invert_;
}

std::size_t Line::point_count() const noexcept {
  return impl_->points_.size();
}

}  // namespace lv
