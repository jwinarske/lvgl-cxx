// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — widgets/bar.hpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// lv::Bar — progress-bar widget.

#pragma once

#include <memory>

#include "../core/object.hpp"
#include "../core/types.hpp"

namespace lv {

class Bar : public Object {
 public:
  explicit Bar(Object* parent);
  ~Bar() override;

  Bar& set_value(int32_t value, AnimEnable anim = AnimEnable::Off);
  Bar& set_start_value(int32_t value, AnimEnable anim = AnimEnable::Off);
  Bar& set_range(int32_t min, int32_t max);
  Bar& set_mode(BarMode mode);

  [[nodiscard]] int32_t value() const noexcept;
  [[nodiscard]] int32_t start_value() const noexcept;
  [[nodiscard]] int32_t min_value() const noexcept;
  [[nodiscard]] int32_t max_value() const noexcept;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace lv
