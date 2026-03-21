// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — widgets/imagebutton.hpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// lv::ImageButton — button widget with per-state image sources.

#pragma once

#include <memory>

#include "../core/object.hpp"
#include "../core/types.hpp"

namespace lv {

class ImageButton : public Object {
 public:
  explicit ImageButton(Object* parent);
  ~ImageButton() override;

  ImageButton& set_src(ObjState state, const void* src);
  [[nodiscard]] const void* src(ObjState state) const noexcept;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace lv
