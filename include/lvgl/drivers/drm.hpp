// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — drivers/drm.hpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// DRM/KMS display driver for Linux framebuffer rendering.
// Requires libdrm at link time.

#pragma once

#include <cstdint>
#include <memory>
#include <string_view>

#include "../core/display.hpp"
#include "concepts.hpp"

namespace lv::drivers {

class DrmDisplay {
 public:
  explicit DrmDisplay(std::string_view device = "/dev/dri/card0");
  ~DrmDisplay();

  DrmDisplay(const DrmDisplay&) = delete;
  DrmDisplay& operator=(const DrmDisplay&) = delete;

  [[nodiscard]] auto flush_cb() {
    return [this](Display& disp, const Area& area,
                  const BufferView2D<Display::Pixel> view) {
      flush(disp, area, view);
    };
  }

  [[nodiscard]] std::span<Display::Pixel> buffer() noexcept;

  [[nodiscard]] int32_t width() const noexcept;
  [[nodiscard]] int32_t height() const noexcept;

  void init();
  void deinit();

 private:
  void flush(Display& disp,
             const Area& area,
             BufferView2D<Display::Pixel> view);

  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace lv::drivers
