// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — src/widgets/window.cpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// Implementation of lv::Window.

#include "lvgl/widgets/window.hpp"
#include "lvgl/widgets/container.hpp"

#include <string>

namespace lv {

struct Window::Impl {
  int32_t header_height_ = 40;
  std::string title_;
  Object* content_ = nullptr;
};

Window::Window(Object* parent)
    : Object(parent), impl_(std::make_unique<Impl>()) {
  // Create the content container as a child.
  impl_->content_ = &create<Container>();
}

Window::~Window() = default;

Window& Window::set_title(std::string_view title) {
  impl_->title_ = title;
  invalidate();
  return *this;
}

Object& Window::add_btn(std::string_view /*icon*/, int32_t /*w*/) {
  auto& btn = create<Container>();
  btn.add_flag(ObjFlags::Clickable);
  return btn;
}

Object& Window::content() {
  return *impl_->content_;
}

std::string_view Window::title() const noexcept {
  return impl_->title_;
}

}  // namespace lv
