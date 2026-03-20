// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Joel Winarske
//
// Port of lv_example_get_started_2 — Button with Click Counter
// A button at (10,10) with size 120x50. Clicking increments a counter label.

#include <string>

#include "sdl2_harness.hpp"

#include "lvgl/widgets/button.hpp"
#include "lvgl/widgets/label.hpp"

#if HAS_SDL2

int main() {
  constexpr int32_t DISP_W = 480;
  constexpr int32_t DISP_H = 320;

  sdl2::init(DISP_W, DISP_H, "Get Started 2 — Button Counter");

  std::vector<lv::Display::Pixel> buf(
      static_cast<std::size_t>(DISP_W) * static_cast<std::size_t>(DISP_H));
  lv::Display display{DISP_W, DISP_H, sdl2::flush_cb};
  display.set_draw_buffers(buf);
  display.set_render_mode(lv::RenderMode::Full);

  // Screen
  auto& scr = display.active_screen();

  // Button style
  static lv::Style btn_style;
  btn_style.set(lv::prop::BgColor{}, lv::Color::from_hex(0x2196F3))
      .set(lv::prop::Radius{}, 8);

  // Button
  auto& btn = scr.create<lv::Button>();
  btn.set_size(120, 50);
  btn.align(lv::Align::TopLeft, 10, 10);
  btn.add_style(btn_style);

  // Label inside button
  static lv::Style lbl_style;
  lbl_style.set(lv::prop::TextColor{}, lv::Color::White());

  auto& lbl = btn.create<lv::Label>();
  lbl.add_style(lbl_style);
  lbl.set_text(std::string_view{"Button: 0"});
  lbl.align(lv::Align::Center);

  // Click handler
  static int click_count = 0;
  btn.on(lv::EventCode::Clicked, [&lbl](lv::Event&) {
       ++click_count;
       lbl.set_text("Button: " + std::to_string(click_count));
       lbl.align(lv::Align::Center);
       std::printf("Click #%d\n", click_count);
     })
      .release();

  // Initial render
  display.refresh();

  // Main loop with hit-test input
  lv::Ticker ticker;
  while (sdl2::ctx.running) {
    sdl2::poll_events();

    if (sdl2::ctx.mouse_clicked &&
        sdl2::hit_test(btn, sdl2::ctx.mouse_x, sdl2::ctx.mouse_y)) {
      btn.send_event(lv::EventCode::Clicked);
    }

    ticker.update();
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
