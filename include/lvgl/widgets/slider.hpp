// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — widgets/slider.hpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// lv::Slider — draggable slider widget with optional range mode.

#pragma once

#include <memory>

#include "../core/object.hpp"
#include "../core/types.hpp"

namespace lv {

class Slider : public Object {
 public:
  explicit Slider(Object* parent);
  ~Slider() override;

  Slider& set_value(int32_t value, AnimEnable anim = AnimEnable::Off);
  Slider& set_left_value(int32_t value, AnimEnable anim = AnimEnable::Off);
  Slider& set_range(int32_t min, int32_t max);
  Slider& set_mode(SliderMode mode);

  [[nodiscard]] int32_t value() const noexcept;
  [[nodiscard]] int32_t left_value() const noexcept;
  [[nodiscard]] int32_t min_value() const noexcept;
  [[nodiscard]] int32_t max_value() const noexcept;
  [[nodiscard]] SliderMode mode() const noexcept;
  [[nodiscard]] bool is_dragged() const noexcept;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace lv
