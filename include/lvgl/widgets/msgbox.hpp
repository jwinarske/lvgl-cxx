// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — widgets/msgbox.hpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// lv::MsgBox — modal / non-modal message box widget.

#pragma once

#include <memory>
#include <string_view>

#include "../core/object.hpp"

namespace lv {

class MsgBox : public Object {
 public:
  explicit MsgBox(Object* parent);
  ~MsgBox() override;

  MsgBox& set_title(std::string_view title);
  MsgBox& set_text(std::string_view text);
  MsgBox& add_btn(std::string_view text);
  MsgBox& set_close_btn(bool en);

  [[nodiscard]] std::string_view title() const noexcept;
  [[nodiscard]] std::string_view text() const noexcept;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace lv
