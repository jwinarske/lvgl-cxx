// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — widgets/scale.hpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// lv::Scale — linear or radial scale with tick marks and labels.

#pragma once

#include <cstdint>
#include <memory>

#include "../core/object.hpp"
#include "../core/types.hpp"

namespace lv {

class Scale : public Object {
 public:
  explicit Scale(Object* parent);
  ~Scale() override;

  Scale& set_mode(ScaleMode mode);
  Scale& set_range(int32_t min, int32_t max);
  Scale& set_total_tick_count(int32_t count);
  Scale& set_major_tick_every(int32_t nth);
  Scale& set_label_show(bool show);

  [[nodiscard]] ScaleMode mode() const noexcept;
  [[nodiscard]] int32_t min_value() const noexcept;
  [[nodiscard]] int32_t max_value() const noexcept;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace lv
