// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — widgets/container.hpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// lv::Container — scrollable box container widget.

#pragma once

#include "../core/object.hpp"

namespace lv {

class Container : public Object {
 public:
  explicit Container(Object* parent);
  ~Container() override;
};

}  // namespace lv
