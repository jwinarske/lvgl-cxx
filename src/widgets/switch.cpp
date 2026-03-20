// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — src/widgets/switch.cpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// Implementation of lv::Switch.

#include "lvgl/widgets/switch.hpp"

namespace lv {

Switch::Switch(Object* parent) : Object(parent) {
  add_flag(ObjFlags::Clickable | ObjFlags::Checkable);
}

Switch::~Switch() = default;

Switch& Switch::set_checked(bool checked, AnimEnable /*anim*/) {
  if (checked) {
    add_state(ObjState::Checked);
  } else {
    remove_state(ObjState::Checked);
  }
  return *this;
}

bool Switch::is_checked() const noexcept {
  return has_state(ObjState::Checked);
}

}  // namespace lv
