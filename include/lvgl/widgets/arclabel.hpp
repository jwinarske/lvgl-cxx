// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — widgets/arclabel.hpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// lv::ArcLabel — text rendered along an arc path.

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

#include "../core/object.hpp"
#include "../core/types.hpp"

namespace lv {

class ArcLabel : public Object {
 public:
  explicit ArcLabel(Object* parent);
  ~ArcLabel() override;

  ArcLabel& set_text(std::string_view text);
  ArcLabel& set_radius(int32_t radius);
  ArcLabel& set_start_angle(int32_t angle);

  [[nodiscard]] std::string_view text() const noexcept;
  [[nodiscard]] int32_t radius() const noexcept;
  [[nodiscard]] int32_t start_angle() const noexcept;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace lv
