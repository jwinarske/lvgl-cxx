// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Joel Winarske
//
// Port of lv_example_get_started_1 — "Hello World"
// Dark blue screen with a white centered label.

#include "sdl2_harness.hpp"

#include "lvgl/widgets/label.hpp"

void create_ui(lv::Display& display) {
  // Dark blue screen background
  static lv::Style scr_style;
  scr_style.set(lv::prop::BgColor{}, lv::Color::from_hex(0x003a57));
  auto& scr = display.active_screen();
  scr.add_style(scr_style);

  // White label centered on screen
  static lv::Style lbl_style;
  lbl_style.set(lv::prop::TextColor{}, lv::Color::White());
  auto& label = scr.create<lv::Label>();
  label.add_style(lbl_style);
  label.set_text(std::string_view{"Hello world"});
  label.align(lv::Align::Center);
}

#if HAS_SDL2

int main() {
  return sdl2::run("Get Started 1 — Hello World", 480, 320, create_ui);
}

#else

#include <cstdio>

int main() {
  std::puts("SDL2 not available. Install libsdl2-dev and rebuild.");
  return 1;
}

#endif
