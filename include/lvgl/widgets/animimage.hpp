// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — widgets/animimage.hpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// lv::AnimImage — animated image widget cycling through source frames.

#pragma once

#include <cstdint>
#include <memory>
#include <span>

#include "image.hpp"

namespace lv {

class AnimImage : public Image {
 public:
  explicit AnimImage(Object* parent);
  ~AnimImage() override;

  AnimImage& set_src(std::span<const void* const> srcs);
  AnimImage& set_duration(uint32_t ms);
  AnimImage& set_repeat(bool en);

  [[nodiscard]] uint32_t src_count() const noexcept;
  [[nodiscard]] uint32_t duration() const noexcept;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace lv
