// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — src/widgets/container.cpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// Implementation of lv::Container.

#include "lvgl/widgets/container.hpp"

namespace lv {

Container::Container(Object* parent) : Object(parent) {
  add_flag(ObjFlags::Scrollable);
}

Container::~Container() = default;

}  // namespace lv
