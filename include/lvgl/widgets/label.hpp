// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — widgets/label.hpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// lv::Label — static or scrolling text label widget.

#pragma once

#include <memory>
#include <string>
#include <string_view>

#include "../core/object.hpp"
#include "../core/types.hpp"

namespace lv {

class Label : public Object {
 public:
  explicit Label(Object* parent);
  ~Label() override;

  Label& set_text(std::string_view text);
  Label& set_text(std::string text);
  Label& set_long_mode(LabelLongMode mode);
  Label& set_recolor(bool en);

  [[nodiscard]] std::string_view text() const noexcept;
  [[nodiscard]] LabelLongMode long_mode() const noexcept;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace lv
