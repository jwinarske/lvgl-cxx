// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — core/draw_buffer.hpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// DrawBuffer<Pixel> — 2D pixel buffer with row-major layout.
// Supports non-owning views over caller-managed storage and owning
// allocator-aware buffers.

#pragma once

#include <cassert>
#include <cstdint>
#include <memory>
#include <vector>

namespace lv {

template <typename Pixel>
class DrawBuffer {
 public:
  // Non-owning view over caller-managed storage
  DrawBuffer(Pixel* data, int32_t w, int32_t h) noexcept
      : data_(data), width_(w), height_(h) {}

  // Owning buffer via allocator
  template <typename Alloc = std::allocator<Pixel>>
  static DrawBuffer allocate(int32_t w, int32_t h, Alloc alloc = {}) {
    auto storage = std::make_shared<std::vector<Pixel, Alloc>>(
        static_cast<std::size_t>(w) * static_cast<std::size_t>(h), Pixel{},
        alloc);
    DrawBuffer buf(storage->data(), w, h);
    buf.storage_ = std::move(storage);
    return buf;
  }

  // C++23 multidimensional subscript operator
  Pixel& operator[](int32_t row, int32_t col) noexcept {
    assert(row >= 0 && row < height_ && "DrawBuffer: row out of bounds");
    assert(col >= 0 && col < width_ && "DrawBuffer: col out of bounds");
    return data_[row * width_ + col];
  }
  const Pixel& operator[](int32_t row, int32_t col) const noexcept {
    assert(row >= 0 && row < height_ && "DrawBuffer: row out of bounds");
    assert(col >= 0 && col < width_ && "DrawBuffer: col out of bounds");
    return data_[row * width_ + col];
  }

  [[nodiscard]] int32_t width() const noexcept { return width_; }
  [[nodiscard]] int32_t height() const noexcept { return height_; }

  [[nodiscard]] Pixel* data() noexcept { return data_; }
  [[nodiscard]] const Pixel* data() const noexcept { return data_; }

  // Fill entire buffer with a single pixel value
  void fill(Pixel value) noexcept {
    const auto sz =
        static_cast<std::size_t>(width_) * static_cast<std::size_t>(height_);
    for (std::size_t i = 0; i < sz; ++i)
      data_[i] = value;
  }

 private:
  Pixel* data_ = nullptr;
  int32_t width_ = 0;
  int32_t height_ = 0;
  // Shared ownership when using allocate()
  std::shared_ptr<void> storage_;
};

}  // namespace lv
