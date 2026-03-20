// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — widgets/checkbox.hpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// lv::Checkbox — checkbox with text label.

#pragma once

#include <memory>
#include <string>
#include <string_view>

#include "../core/object.hpp"
#include "../core/types.hpp"

namespace lv {

class Checkbox : public Object {
 public:
  explicit Checkbox(Object* parent);
  ~Checkbox() override;

  Checkbox& set_text(std::string_view text);
  Checkbox& set_text(std::string text);

  [[nodiscard]] std::string_view text() const noexcept;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace lv
