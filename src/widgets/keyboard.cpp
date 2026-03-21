// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — src/widgets/keyboard.cpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// Implementation of lv::Keyboard.

#include "lvgl/widgets/keyboard.hpp"

namespace lv {

class TextArea;

struct Keyboard::Impl {
  KeyboardMode mode_ = KeyboardMode::Text;
  TextArea* textarea_ = nullptr;
};

Keyboard::Keyboard(Object* parent)
    : Object(parent), impl_(std::make_unique<Impl>()) {
  add_flag(ObjFlags::Clickable);
}

Keyboard::~Keyboard() = default;

Keyboard& Keyboard::set_textarea(TextArea& ta) {
  impl_->textarea_ = &ta;
  invalidate();
  return *this;
}

Keyboard& Keyboard::set_mode(KeyboardMode mode) {
  impl_->mode_ = mode;
  invalidate();
  return *this;
}

TextArea* Keyboard::textarea() const noexcept {
  return impl_->textarea_;
}

KeyboardMode Keyboard::mode() const noexcept {
  return impl_->mode_;
}

}  // namespace lv
