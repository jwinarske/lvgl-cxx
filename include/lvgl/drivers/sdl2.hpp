// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — drivers/sdl2.hpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// SDL2 display + mouse/keyboard driver.
// Requires SDL2 to be available at link time.
// This is a platform driver — only compiled when SDL2 is present.

#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string_view>

#include "../core/display.hpp"
#include "concepts.hpp"
#include "input_device.hpp"

namespace lv::drivers {

// ── Sdl2Display ─────────────────────────────────────────────────────────────
// Creates an SDL2 window and provides flush callback and input polling.
//
// Usage:
//   lv::drivers::Sdl2Display sdl{480, 320, "My App"};
//   lv::Display display{480, 320, sdl.flush_cb()};
//   display.set_draw_buffers(sdl.buffer());
//   auto& pointer = sdl.pointer();
//   while (sdl.running()) {
//     sdl.poll_events();
//     pointer.poll();
//     display.refresh();
//   }

class Sdl2Display {
 public:
  Sdl2Display(int32_t w, int32_t h, std::string_view title = "lvgl-cxx");
  ~Sdl2Display();

  Sdl2Display(const Sdl2Display&) = delete;
  Sdl2Display& operator=(const Sdl2Display&) = delete;

  // Flush callback suitable for Display constructor
  [[nodiscard]] auto flush_cb() {
    return
        [this](Display& disp, const Area& area,
               BufferView2D<Display::Pixel> view) { flush(disp, area, view); };
  }

  // Draw buffer — managed by this driver, sized to full screen
  [[nodiscard]] std::span<Display::Pixel> buffer() noexcept;

  // Input devices
  [[nodiscard]] Pointer& pointer() noexcept;
  [[nodiscard]] Keypad& keypad() noexcept;

  // Event loop helpers
  void poll_events();
  [[nodiscard]] bool running() const noexcept;
  void quit() noexcept;

  [[nodiscard]] int32_t width() const noexcept;
  [[nodiscard]] int32_t height() const noexcept;

 private:
  void flush(Display& disp,
             const Area& area,
             BufferView2D<Display::Pixel> view);

  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace lv::drivers
