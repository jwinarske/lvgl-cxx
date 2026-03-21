// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — widgets/list.hpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// lv::List — scrollable list widget with text and button items.

#pragma once

#include <memory>
#include <string_view>

#include "../core/object.hpp"

namespace lv {

class List : public Object {
 public:
  explicit List(Object* parent);
  ~List() override;

  Object& add_text(std::string_view text);
  Object& add_btn(std::string_view icon, std::string_view text);
};

}  // namespace lv
