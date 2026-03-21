// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — widgets/tabview.hpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// lv::TabView — tab-based container widget.

#pragma once

#include <cstdint>
#include <memory>
#include <string_view>

#include "../core/object.hpp"
#include "../core/types.hpp"

namespace lv {

class TabView : public Object {
 public:
  explicit TabView(Object* parent);
  ~TabView() override;

  Object& add_tab(std::string_view name);
  TabView& set_active(uint32_t idx, AnimEnable anim = AnimEnable::Off);

  [[nodiscard]] uint32_t active_tab() const noexcept;
  [[nodiscard]] uint32_t tab_count() const noexcept;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace lv
