// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — widgets/image.hpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// lv::Image — image display widget with transform support.

#pragma once

#include <memory>
#include <string>
#include <string_view>

#include "../core/object.hpp"
#include "../core/types.hpp"

namespace lv {

class Image : public Object {
 public:
  explicit Image(Object* parent);
  ~Image() override;

  Image& set_src(const void* src);
  Image& set_src(std::string_view path);
  Image& set_offset(int32_t x, int32_t y);
  Image& set_scale(uint32_t zoom);
  Image& set_scale_x(uint32_t zoom);
  Image& set_scale_y(uint32_t zoom);
  Image& set_rotation(int32_t angle);
  Image& set_pivot(int32_t x, int32_t y);
  Image& set_blend_mode(BlendMode mode);
  Image& set_antialias(bool en);
  Image& set_inner_align(ImageAlign align);

  [[nodiscard]] const void* src() const noexcept;
  [[nodiscard]] int32_t offset_x() const noexcept;
  [[nodiscard]] int32_t offset_y() const noexcept;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace lv
