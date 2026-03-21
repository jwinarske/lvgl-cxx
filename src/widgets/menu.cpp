// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — src/widgets/menu.cpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// Implementation of lv::Menu.

#include "lvgl/widgets/menu.hpp"
#include "lvgl/widgets/container.hpp"

namespace lv {

struct Menu::Impl {
  Object* main_page_ = nullptr;
  Object* sidebar_page_ = nullptr;
  bool sidebar_visible_ = false;
};

Menu::Menu(Object* parent) : Object(parent), impl_(std::make_unique<Impl>()) {}

Menu::~Menu() = default;

Object& Menu::create_page() {
  auto& page = create<Container>();
  return page;
}

Menu& Menu::set_page(Object& page) {
  impl_->main_page_ = &page;
  invalidate();
  return *this;
}

Menu& Menu::set_sidebar_page(Object& page) {
  impl_->sidebar_page_ = &page;
  invalidate();
  return *this;
}

Menu& Menu::set_sidebar_visible(bool visible) {
  impl_->sidebar_visible_ = visible;
  invalidate();
  return *this;
}

bool Menu::is_sidebar_visible() const noexcept {
  return impl_->sidebar_visible_;
}

}  // namespace lv
