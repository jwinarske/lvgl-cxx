// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — widgets/keyboard.hpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// lv::Keyboard — on-screen keyboard widget.

#pragma once

#include <memory>

#include "../core/object.hpp"
#include "../core/types.hpp"

namespace lv {

class TextArea;

class Keyboard : public Object {
 public:
  explicit Keyboard(Object* parent);
  ~Keyboard() override;

  Keyboard& set_textarea(TextArea& ta);
  Keyboard& set_mode(KeyboardMode mode);

  [[nodiscard]] TextArea* textarea() const noexcept;
  [[nodiscard]] KeyboardMode mode() const noexcept;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace lv
