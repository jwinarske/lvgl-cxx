// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — src/widgets/animimage.cpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// Implementation of lv::AnimImage.

#include "lvgl/widgets/animimage.hpp"

#include <vector>

namespace lv {

struct AnimImage::Impl {
  std::vector<const void*> srcs_;
  uint32_t duration_ms_ = 1000;
  bool repeat_ = true;
};

AnimImage::AnimImage(Object* parent)
    : Image(parent), impl_(std::make_unique<Impl>()) {}

AnimImage::~AnimImage() = default;

AnimImage& AnimImage::set_src(std::span<const void* const> srcs) {
  impl_->srcs_.assign(srcs.begin(), srcs.end());
  if (!impl_->srcs_.empty())
    Image::set_src(impl_->srcs_.front());
  invalidate();
  return *this;
}

AnimImage& AnimImage::set_duration(uint32_t ms) {
  impl_->duration_ms_ = ms;
  return *this;
}

AnimImage& AnimImage::set_repeat(bool en) {
  impl_->repeat_ = en;
  return *this;
}

uint32_t AnimImage::src_count() const noexcept {
  return static_cast<uint32_t>(impl_->srcs_.size());
}

uint32_t AnimImage::duration() const noexcept {
  return impl_->duration_ms_;
}

}  // namespace lv
