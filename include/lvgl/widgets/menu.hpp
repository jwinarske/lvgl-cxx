// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — widgets/menu.hpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// lv::Menu — hierarchical menu widget.

#pragma once

#include <memory>

#include "../core/object.hpp"

namespace lv {

class Menu : public Object {
 public:
  explicit Menu(Object* parent);
  ~Menu() override;

  Object& create_page();
  Menu& set_page(Object& page);
  Menu& set_sidebar_page(Object& page);
  Menu& set_sidebar_visible(bool visible);

  [[nodiscard]] bool is_sidebar_visible() const noexcept;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace lv
