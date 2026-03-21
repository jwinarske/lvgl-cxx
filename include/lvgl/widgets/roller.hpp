// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — widgets/roller.hpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// lv::Roller — scrollable option picker with visible rows.

#pragma once

#include <memory>
#include <string>
#include <string_view>

#include "../core/object.hpp"
#include "../core/types.hpp"

namespace lv {

class Roller : public Object {
 public:
  explicit Roller(Object* parent);
  ~Roller() override;

  Roller& set_options(std::string_view options,
                      RollerMode mode = RollerMode::Normal);
  Roller& set_selected(uint32_t index, AnimEnable anim = AnimEnable::Off);
  Roller& set_visible_row_count(uint32_t count);

  [[nodiscard]] uint32_t selected() const noexcept;
  [[nodiscard]] std::string selected_str() const;
  [[nodiscard]] uint32_t option_count() const noexcept;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace lv
