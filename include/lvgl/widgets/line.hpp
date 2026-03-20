// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — widgets/line.hpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// lv::Line — polyline widget drawn from a sequence of points.

#pragma once

#include <cstdint>
#include <memory>
#include <span>

#include "../core/object.hpp"
#include "../misc/area.hpp"

namespace lv {

class Line : public Object {
 public:
  explicit Line(Object* parent);
  ~Line() override;

  Line& set_points(std::span<const Point> pts);
  Line& set_y_invert(bool inv);

  [[nodiscard]] bool y_invert() const noexcept;
  [[nodiscard]] std::size_t point_count() const noexcept;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace lv
