// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — widgets/canvas.hpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// lv::Canvas — pixel-level drawing surface widget.

#pragma once

#include <cstdint>
#include <memory>

#include "../core/object.hpp"
#include "../misc/color.hpp"

namespace lv {

class Canvas : public Object {
 public:
  explicit Canvas(Object* parent);
  ~Canvas() override;

  Canvas& set_buffer(void* buf, int32_t w, int32_t h);
  Canvas& fill_bg(Color color, uint8_t opa = 255);
  Canvas& set_px(int32_t x, int32_t y, Color color);

  [[nodiscard]] int32_t width() const noexcept;
  [[nodiscard]] int32_t height() const noexcept;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace lv
