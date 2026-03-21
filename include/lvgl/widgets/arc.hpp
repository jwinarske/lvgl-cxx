// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — widgets/arc.hpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// lv::Arc — arc (circular gauge) widget.

#pragma once

#include <memory>

#include "../core/object.hpp"
#include "../core/types.hpp"

namespace lv {

class Arc : public Object {
 public:
  explicit Arc(Object* parent);
  ~Arc() override;

  Arc& set_start_angle(int32_t angle);
  Arc& set_end_angle(int32_t angle);
  Arc& set_bg_start_angle(int32_t angle);
  Arc& set_bg_end_angle(int32_t angle);
  Arc& set_value(int32_t value);
  Arc& set_range(int32_t min, int32_t max);
  Arc& set_mode(ArcMode mode);
  Arc& set_rotation(int32_t rotation);

  [[nodiscard]] int32_t angle_start() const noexcept;
  [[nodiscard]] int32_t angle_end() const noexcept;
  [[nodiscard]] int32_t bg_angle_start() const noexcept;
  [[nodiscard]] int32_t bg_angle_end() const noexcept;
  [[nodiscard]] int32_t value() const noexcept;
  [[nodiscard]] int32_t min_value() const noexcept;
  [[nodiscard]] int32_t max_value() const noexcept;
  [[nodiscard]] int32_t rotation() const noexcept;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace lv
