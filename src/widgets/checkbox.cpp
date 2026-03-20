// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — src/widgets/checkbox.cpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// Implementation of lv::Checkbox.

#include "lvgl/widgets/checkbox.hpp"

#include <utility>

namespace lv {

struct Checkbox::Impl {
  std::string text_;
};

Checkbox::Checkbox(Object* parent)
    : Object(parent), impl_(std::make_unique<Impl>()) {
  add_flag(ObjFlags::Clickable | ObjFlags::Checkable);
}

Checkbox::~Checkbox() = default;

Checkbox& Checkbox::set_text(std::string_view text) {
  impl_->text_ = text;
  invalidate();
  return *this;
}

Checkbox& Checkbox::set_text(std::string text) {
  impl_->text_ = std::move(text);
  invalidate();
  return *this;
}

std::string_view Checkbox::text() const noexcept {
  return impl_->text_;
}

}  // namespace lv
