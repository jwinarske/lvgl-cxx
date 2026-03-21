// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — src/widgets/msgbox.cpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// Implementation of lv::MsgBox.

#include "lvgl/widgets/msgbox.hpp"

#include <string>
#include <vector>

namespace lv {

struct MsgBox::Impl {
  std::string title_;
  std::string text_;
  std::vector<std::string> btn_texts_;
  bool close_btn_ = true;
};

MsgBox::MsgBox(Object* parent)
    : Object(parent), impl_(std::make_unique<Impl>()) {}

MsgBox::~MsgBox() = default;

MsgBox& MsgBox::set_title(std::string_view title) {
  impl_->title_ = title;
  invalidate();
  return *this;
}

MsgBox& MsgBox::set_text(std::string_view text) {
  impl_->text_ = text;
  invalidate();
  return *this;
}

MsgBox& MsgBox::add_btn(std::string_view text) {
  impl_->btn_texts_.emplace_back(text);
  invalidate();
  return *this;
}

MsgBox& MsgBox::set_close_btn(bool en) {
  impl_->close_btn_ = en;
  invalidate();
  return *this;
}

std::string_view MsgBox::title() const noexcept {
  return impl_->title_;
}

std::string_view MsgBox::text() const noexcept {
  return impl_->text_;
}

}  // namespace lv
