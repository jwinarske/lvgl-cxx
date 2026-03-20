// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — src/widgets/image.cpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// Implementation of lv::Image.

#include "lvgl/widgets/image.hpp"

namespace lv {

struct Image::Impl {
  const void* src_ = nullptr;
  std::string src_path_;
  int32_t offset_x_ = 0;
  int32_t offset_y_ = 0;
  uint32_t scale_ = 256;
  uint32_t scale_x_ = 256;
  uint32_t scale_y_ = 256;
  int32_t rotation_ = 0;
  int32_t pivot_x_ = 0;
  int32_t pivot_y_ = 0;
  BlendMode blend_ = BlendMode::Normal;
  bool antialias_ = false;
  ImageAlign inner_align_ = ImageAlign::Default;
};

Image::Image(Object* parent)
    : Object(parent), impl_(std::make_unique<Impl>()) {}

Image::~Image() = default;

Image& Image::set_src(const void* src) {
  impl_->src_ = src;
  impl_->src_path_.clear();
  invalidate();
  return *this;
}

Image& Image::set_src(std::string_view path) {
  impl_->src_path_ = path;
  impl_->src_ = impl_->src_path_.c_str();
  invalidate();
  return *this;
}

Image& Image::set_offset(int32_t x, int32_t y) {
  impl_->offset_x_ = x;
  impl_->offset_y_ = y;
  invalidate();
  return *this;
}

Image& Image::set_scale(uint32_t zoom) {
  impl_->scale_ = zoom;
  impl_->scale_x_ = zoom;
  impl_->scale_y_ = zoom;
  invalidate();
  return *this;
}

Image& Image::set_scale_x(uint32_t zoom) {
  impl_->scale_x_ = zoom;
  invalidate();
  return *this;
}

Image& Image::set_scale_y(uint32_t zoom) {
  impl_->scale_y_ = zoom;
  invalidate();
  return *this;
}

Image& Image::set_rotation(int32_t angle) {
  impl_->rotation_ = angle;
  invalidate();
  return *this;
}

Image& Image::set_pivot(int32_t x, int32_t y) {
  impl_->pivot_x_ = x;
  impl_->pivot_y_ = y;
  invalidate();
  return *this;
}

Image& Image::set_blend_mode(BlendMode mode) {
  impl_->blend_ = mode;
  invalidate();
  return *this;
}

Image& Image::set_antialias(bool en) {
  impl_->antialias_ = en;
  invalidate();
  return *this;
}

Image& Image::set_inner_align(ImageAlign align) {
  impl_->inner_align_ = align;
  invalidate();
  return *this;
}

const void* Image::src() const noexcept {
  return impl_->src_;
}

int32_t Image::offset_x() const noexcept {
  return impl_->offset_x_;
}

int32_t Image::offset_y() const noexcept {
  return impl_->offset_y_;
}

}  // namespace lv
