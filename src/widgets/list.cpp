// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — src/widgets/list.cpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// Implementation of lv::List.

#include "lvgl/widgets/list.hpp"
#include "lvgl/widgets/container.hpp"

namespace lv {

List::List(Object* parent) : Object(parent) {
  add_flag(ObjFlags::Scrollable);
}

List::~List() = default;

Object& List::add_text(std::string_view /*text*/) {
  auto& item = create<Container>();
  return item;
}

Object& List::add_btn(std::string_view /*icon*/, std::string_view /*text*/) {
  auto& btn = create<Container>();
  btn.add_flag(ObjFlags::Clickable);
  return btn;
}

}  // namespace lv
