// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — src/widgets/textarea.cpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// Implementation of lv::TextArea.

#include "lvgl/widgets/textarea.hpp"

#include <algorithm>

namespace lv {

struct TextArea::Impl {
  std::string text_;
  std::string placeholder_;
  std::string accepted_chars_;
  uint32_t max_length_ = 0;
  int32_t cursor_pos_ = 0;
  bool password_mode_ = false;
  bool one_line_ = false;
};

TextArea::TextArea(Object* parent)
    : Object(parent), impl_(std::make_unique<Impl>()) {
  add_flag(ObjFlags::Clickable | ObjFlags::ClickFocusable);
}

TextArea::~TextArea() = default;

TextArea& TextArea::set_text(std::string_view text) {
  impl_->text_ = text;
  impl_->cursor_pos_ = static_cast<int32_t>(impl_->text_.size());
  invalidate();
  return *this;
}

TextArea& TextArea::add_char(uint32_t c) {
  if (impl_->max_length_ > 0 && impl_->text_.size() >= impl_->max_length_)
    return *this;
  if (!impl_->accepted_chars_.empty() &&
      impl_->accepted_chars_.find(static_cast<char>(c)) == std::string::npos)
    return *this;
  auto pos = static_cast<std::size_t>(std::clamp<int32_t>(
      impl_->cursor_pos_, 0, static_cast<int32_t>(impl_->text_.size())));
  impl_->text_.insert(impl_->text_.begin() + static_cast<std::ptrdiff_t>(pos),
                      static_cast<char>(c));
  ++impl_->cursor_pos_;
  invalidate();
  return *this;
}

TextArea& TextArea::add_text(std::string_view text) {
  for (char c : text)
    add_char(static_cast<uint32_t>(c));
  return *this;
}

TextArea& TextArea::delete_char() {
  if (impl_->cursor_pos_ <= 0 || impl_->text_.empty())
    return *this;
  auto pos = static_cast<std::size_t>(impl_->cursor_pos_ - 1);
  if (pos < impl_->text_.size())
    impl_->text_.erase(pos, 1);
  --impl_->cursor_pos_;
  invalidate();
  return *this;
}

TextArea& TextArea::set_placeholder_text(std::string_view text) {
  impl_->placeholder_ = text;
  invalidate();
  return *this;
}

TextArea& TextArea::set_accepted_chars(std::string_view chars) {
  impl_->accepted_chars_ = chars;
  return *this;
}

TextArea& TextArea::set_max_length(uint32_t len) {
  impl_->max_length_ = len;
  return *this;
}

TextArea& TextArea::set_password_mode(bool en) {
  impl_->password_mode_ = en;
  invalidate();
  return *this;
}

TextArea& TextArea::set_one_line(bool en) {
  impl_->one_line_ = en;
  invalidate();
  return *this;
}

TextArea& TextArea::set_cursor_pos(int32_t pos) {
  impl_->cursor_pos_ =
      std::clamp<int32_t>(pos, 0, static_cast<int32_t>(impl_->text_.size()));
  invalidate();
  return *this;
}

std::string_view TextArea::text() const noexcept {
  return impl_->text_;
}

int32_t TextArea::cursor_pos() const noexcept {
  return impl_->cursor_pos_;
}

bool TextArea::is_password() const noexcept {
  return impl_->password_mode_;
}

}  // namespace lv
