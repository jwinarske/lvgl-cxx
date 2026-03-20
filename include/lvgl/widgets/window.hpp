// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — widgets/window.hpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// lv::Window — window widget with header and content area.

#pragma once

#include <cstdint>
#include <memory>
#include <string_view>

#include "../core/object.hpp"

namespace lv {

class Window : public Object {
 public:
  explicit Window(Object* parent);
  ~Window() override;

  Window& set_title(std::string_view title);
  Object& add_btn(std::string_view icon, int32_t w);
  Object& content();

  [[nodiscard]] std::string_view title() const noexcept;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace lv
