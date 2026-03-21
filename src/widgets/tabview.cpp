// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — src/widgets/tabview.cpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// Implementation of lv::TabView.

#include "lvgl/widgets/tabview.hpp"
#include "lvgl/widgets/container.hpp"

#include <string>
#include <vector>

namespace lv {

struct TabView::Impl {
  Dir tab_pos_ = Dir::Top;
  int32_t tab_size_ = 40;
  uint32_t active_tab_ = 0;
  std::vector<std::string> tab_names_;
};

TabView::TabView(Object* parent)
    : Object(parent), impl_(std::make_unique<Impl>()) {}

TabView::~TabView() = default;

Object& TabView::add_tab(std::string_view name) {
  impl_->tab_names_.emplace_back(name);
  auto& tab = create<Container>();
  return tab;
}

TabView& TabView::set_active(uint32_t idx, AnimEnable /*anim*/) {
  impl_->active_tab_ = idx;
  invalidate();
  return *this;
}

uint32_t TabView::active_tab() const noexcept {
  return impl_->active_tab_;
}

uint32_t TabView::tab_count() const noexcept {
  return static_cast<uint32_t>(impl_->tab_names_.size());
}

}  // namespace lv
