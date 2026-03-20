// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — widgets/switch.hpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// lv::Switch — toggle switch widget (uses ObjState::Checked).

#pragma once

#include "../core/object.hpp"
#include "../core/types.hpp"

namespace lv {

class Switch : public Object {
 public:
  explicit Switch(Object* parent);
  ~Switch() override;

  Switch& set_checked(bool checked, AnimEnable anim = AnimEnable::On);

  [[nodiscard]] bool is_checked() const noexcept;
};

}  // namespace lv
