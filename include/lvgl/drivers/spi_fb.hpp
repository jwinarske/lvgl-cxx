// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — drivers/spi_fb.hpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// SPI/I2C framebuffer stubs for embedded display controllers
// (e.g., ILI9341, ST7789, SSD1306).
//
// These are header-only stubs that define the interface.  Platform-specific
// implementations are provided by the user for their specific hardware.

#pragma once

#include <cstdint>
#include <span>

#include "../core/display.hpp"
#include "../misc/area.hpp"
#include "concepts.hpp"

namespace lv::drivers {

// ── SpiFramebuffer ──────────────────────────────────────────────────────────
// User subclasses this and implements the pure virtual transfer methods.
class SpiFramebuffer {
 public:
  SpiFramebuffer(int32_t w, int32_t h) noexcept : width_(w), height_(h) {}
  virtual ~SpiFramebuffer() = default;

  SpiFramebuffer(const SpiFramebuffer&) = delete;
  SpiFramebuffer& operator=(const SpiFramebuffer&) = delete;

  [[nodiscard]] auto flush_cb() {
    return [this](Display& disp, const Area& area,
                  BufferView2D<Display::Pixel> view) {
      set_window(area.x1, area.y1, area.x2, area.y2);
      const auto px_count = static_cast<std::size_t>(view.rows) *
                            static_cast<std::size_t>(view.cols);
      write_pixels({view.data, px_count});
    };
  }

  [[nodiscard]] int32_t width() const noexcept { return width_; }
  [[nodiscard]] int32_t height() const noexcept { return height_; }

 protected:
  // Subclass must implement these for specific hardware
  virtual void set_window(int32_t x1, int32_t y1, int32_t x2, int32_t y2) = 0;
  virtual void write_pixels(std::span<const Display::Pixel> data) = 0;

 private:
  int32_t width_;
  int32_t height_;
};

// ── I2cFramebuffer ──────────────────────────────────────────────────────────
// Same pattern for I2C-connected displays (e.g., SSD1306 OLED).
class I2cFramebuffer {
 public:
  I2cFramebuffer(int32_t w, int32_t h) noexcept : width_(w), height_(h) {}
  virtual ~I2cFramebuffer() = default;

  I2cFramebuffer(const I2cFramebuffer&) = delete;
  I2cFramebuffer& operator=(const I2cFramebuffer&) = delete;

  [[nodiscard]] auto flush_cb() {
    return [this](Display& disp, const Area& area,
                  BufferView2D<Display::Pixel> view) {
      set_window(area.x1, area.y1, area.x2, area.y2);
      const auto px_count = static_cast<std::size_t>(view.rows) *
                            static_cast<std::size_t>(view.cols);
      write_pixels({view.data, px_count});
    };
  }

  [[nodiscard]] int32_t width() const noexcept { return width_; }
  [[nodiscard]] int32_t height() const noexcept { return height_; }

 protected:
  virtual void set_window(int32_t x1, int32_t y1, int32_t x2, int32_t y2) = 0;
  virtual void write_pixels(std::span<const Display::Pixel> data) = 0;

 private:
  int32_t width_;
  int32_t height_;
};

}  // namespace lv::drivers
