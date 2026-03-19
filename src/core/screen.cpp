// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — src/core/screen.cpp
// Upstream LVGL baseline: v9.5.0  https://github.com/lvgl/lvgl/releases/tag/v9.5.0

#include "lvgl/core/screen.hpp"

namespace lv {

Screen::Screen(Display* display)
    : Object(nullptr),  // Screen has no parent — it is the root of the tree
      display_(display) {}

Screen::~Screen() = default;

Display* Screen::owner_display() noexcept {
    return display_;
}
const Display* Screen::owner_display() const noexcept {
    return display_;
}

void Screen::on_create() {}

}  // namespace lv
