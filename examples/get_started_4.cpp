// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Joel Winarske
//
// Port of lv_example_get_started_4 — Slider with Value Display
// A slider centered on screen. Click/drag on the slider to change value.
// Label above shows the current value.

#include <string>

#include "sdl2_harness.hpp"

#include "lvgl/widgets/container.hpp"
#include "lvgl/widgets/label.hpp"
#include "lvgl/widgets/slider.hpp"

#if HAS_SDL2

int main() {
  constexpr int32_t W = 480;
  constexpr int32_t H = 320;

  sdl2::init(W, H, "Get Started 4 — Slider with Value");

  std::vector<lv::Display::Pixel> buf(
      static_cast<std::size_t>(W) * static_cast<std::size_t>(H));
  lv::Display display{W, H, sdl2::flush_cb};
  display.set_draw_buffers(buf);
  display.set_render_mode(lv::RenderMode::Full);

  auto& scr = display.active_screen();

  // Screen background
  static lv::Style scr_style;
  scr_style.set(lv::prop::BgColor{}, lv::Color::from_hex(0xF5F5F5));
  scr.add_style(scr_style);

  // Slider track style
  static lv::Style slider_style;
  slider_style.set(lv::prop::BgColor{}, lv::Color::from_hex(0xBBDEFB))
      .set(lv::prop::Radius{}, 6);

  // Slider
  auto& slider = scr.create<lv::Slider>();
  slider.set_size(200, 20);
  slider.align(lv::Align::Center);
  slider.set_range(0, 100);
  slider.set_value(0);
  slider.add_style(slider_style);

  // Value label
  static lv::Style lbl_style;
  lbl_style.set(lv::prop::TextColor{}, lv::Color::from_hex(0x333333));

  auto& label = scr.create<lv::Label>();
  label.add_style(lbl_style);
  label.set_text(std::string_view{"0"});
  label.align(lv::Align::Center, 0, -40);

  // Slider indicator style (filled portion)
  static lv::Style indicator_style;
  indicator_style.set(lv::prop::BgColor{}, lv::Color::from_hex(0x2196F3))
      .set(lv::prop::Radius{}, 6);

  auto& indicator = slider.create<lv::Container>();
  indicator.set_size(0, 20);
  indicator.set_pos(0, 0);
  indicator.add_style(indicator_style);

  // Compute slider absolute position (for hit-testing)
  // Slider is centered: x = (480-200)/2 = 140, y = (320-20)/2 = 150
  const int32_t slider_abs_x = (W - 200) / 2;
  const int32_t slider_abs_y = (H - 20) / 2;
  const int32_t slider_w = 200;

  auto update_slider = [&](int32_t val) {
    slider.set_value(val);
    label.set_text(std::to_string(val));
    label.align(lv::Align::Center, 0, -40);
    // Update indicator width
    indicator.set_size(val * slider_w / 100, 20);
  };

  display.refresh();

  lv::Ticker ticker;
  while (sdl2::ctx.running) {
    sdl2::poll_events();
    ticker.update();

    // Mouse interaction: click or drag on slider to set value
    if (sdl2::ctx.mouse_pressed) {
      const int32_t mx = sdl2::ctx.mouse_x;
      const int32_t my = sdl2::ctx.mouse_y;

      // Check if mouse is within slider area (with some vertical tolerance)
      if (mx >= slider_abs_x && mx <= slider_abs_x + slider_w &&
          my >= slider_abs_y - 10 && my <= slider_abs_y + 30) {
        // Map x position to slider value
        int32_t rel = mx - slider_abs_x;
        int32_t val = rel * 100 / slider_w;
        val = std::clamp(val, int32_t{0}, int32_t{100});
        update_slider(val);
      }
    }

    display.refresh();
    std::this_thread::sleep_for(std::chrono::milliseconds(16));
  }

  sdl2::deinit();
  return 0;
}

#else

#include <cstdio>

int main() {
  std::puts("SDL2 not available. Install libsdl2-dev and rebuild.");
  return 1;
}

#endif
