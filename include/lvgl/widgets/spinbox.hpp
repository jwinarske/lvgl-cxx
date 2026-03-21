// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — widgets/spinbox.hpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// lv::SpinBox — numeric input widget with increment/decrement controls.

#pragma once

#include <cstdint>
#include <memory>

#include "../core/object.hpp"
#include "../core/types.hpp"

namespace lv {

class SpinBox : public Object {
 public:
  explicit SpinBox(Object* parent);
  ~SpinBox() override;

  SpinBox& set_value(int32_t value);
  SpinBox& set_range(int32_t min, int32_t max);
  SpinBox& set_step(int32_t step);
  SpinBox& set_digit_count(uint8_t count);
  SpinBox& set_decimal_point(uint8_t pos);

  SpinBox& increment();
  SpinBox& decrement();

  [[nodiscard]] int32_t value() const noexcept;
  [[nodiscard]] int32_t step() const noexcept;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace lv
