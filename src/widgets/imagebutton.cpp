// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — src/widgets/imagebutton.cpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// Implementation of lv::ImageButton.

#include "lvgl/widgets/imagebutton.hpp"

namespace lv {

struct ImageButton::Impl {
  const void* src_released_ = nullptr;
  const void* src_pressed_ = nullptr;
  const void* src_disabled_ = nullptr;
  const void* src_checked_released_ = nullptr;
  const void* src_checked_pressed_ = nullptr;
};

ImageButton::ImageButton(Object* parent)
    : Object(parent), impl_(std::make_unique<Impl>()) {
  add_flag(ObjFlags::Clickable);
}

ImageButton::~ImageButton() = default;

ImageButton& ImageButton::set_src(ObjState state, const void* src) {
  if (state == ObjState::Default)
    impl_->src_released_ = src;
  else if (state == ObjState::Pressed)
    impl_->src_pressed_ = src;
  else if (state == ObjState::Disabled)
    impl_->src_disabled_ = src;
  else if (state == ObjState::Checked)
    impl_->src_checked_released_ = src;
  else if (state == (ObjState::Checked | ObjState::Pressed))
    impl_->src_checked_pressed_ = src;
  invalidate();
  return *this;
}

const void* ImageButton::src(ObjState state) const noexcept {
  if (state == ObjState::Default)
    return impl_->src_released_;
  if (state == ObjState::Pressed)
    return impl_->src_pressed_;
  if (state == ObjState::Disabled)
    return impl_->src_disabled_;
  if (state == ObjState::Checked)
    return impl_->src_checked_released_;
  if (state == (ObjState::Checked | ObjState::Pressed))
    return impl_->src_checked_pressed_;
  return nullptr;
}

}  // namespace lv
