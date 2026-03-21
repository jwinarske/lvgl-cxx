// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — src/widgets/button.cpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// Implementation of lv::Button.

#include "lvgl/widgets/button.hpp"

namespace lv {

Button::Button(Object* parent) : Object(parent) {
  add_flag(ObjFlags::Clickable | ObjFlags::ClickFocusable);
}

Button::~Button() = default;

}  // namespace lv
