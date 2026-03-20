// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — widgets/textarea.hpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// lv::TextArea — editable text input widget.

#pragma once

#include <memory>
#include <string>
#include <string_view>

#include "../core/object.hpp"
#include "../core/types.hpp"

namespace lv {

class TextArea : public Object {
 public:
  explicit TextArea(Object* parent);
  ~TextArea() override;

  TextArea& set_text(std::string_view text);
  TextArea& add_char(uint32_t c);
  TextArea& add_text(std::string_view text);
  TextArea& delete_char();
  TextArea& set_placeholder_text(std::string_view text);
  TextArea& set_accepted_chars(std::string_view chars);
  TextArea& set_max_length(uint32_t len);
  TextArea& set_password_mode(bool en);
  TextArea& set_one_line(bool en);
  TextArea& set_cursor_pos(int32_t pos);

  [[nodiscard]] std::string_view text() const noexcept;
  [[nodiscard]] int32_t cursor_pos() const noexcept;
  [[nodiscard]] bool is_password() const noexcept;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace lv
