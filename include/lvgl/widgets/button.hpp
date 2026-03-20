// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — widgets/button.hpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// lv::Button — simple clickable button widget.

#pragma once

#include "../core/object.hpp"

namespace lv {

class Button : public Object {
 public:
  explicit Button(Object* parent);
  ~Button() override;
};

}  // namespace lv
