// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — src/widgets/canvas.cpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// Implementation of lv::Canvas.

#include "lvgl/widgets/canvas.hpp"

namespace lv {

struct Canvas::Impl {
  void* buf_ = nullptr;
  int32_t width_ = 0;
  int32_t height_ = 0;
};

Canvas::Canvas(Object* parent)
    : Object(parent), impl_(std::make_unique<Impl>()) {}

Canvas::~Canvas() = default;

Canvas& Canvas::set_buffer(void* buf, int32_t w, int32_t h) {
  impl_->buf_ = buf;
  impl_->width_ = w;
  impl_->height_ = h;
  invalidate();
  return *this;
}

Canvas& Canvas::fill_bg(Color /*color*/, uint8_t /*opa*/) {
  invalidate();
  return *this;
}

Canvas& Canvas::set_px(int32_t /*x*/, int32_t /*y*/, Color /*color*/) {
  invalidate();
  return *this;
}

int32_t Canvas::width() const noexcept {
  return impl_->width_;
}

int32_t Canvas::height() const noexcept {
  return impl_->height_;
}

}  // namespace lv
