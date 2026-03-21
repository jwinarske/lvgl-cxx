// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Joel Winarske
//
// Port of lv_example_get_started_3 — Styled Buttons
// Two buttons with custom styles: a gray rounded button and a red pill button.
// Both have darker pressed-state styles.

#include "sdl2_harness.hpp"

#include "lvgl/widgets/button.hpp"
#include "lvgl/widgets/label.hpp"

namespace {

// ── Button 1 styles: light gray, rounded, black border and text ──
lv::Style style_btn1;
lv::Style style_btn1_pressed;
lv::Style style_lbl1;

// ── Button 2 styles: red pill shape, white text ──
lv::Style style_btn2;
lv::Style style_btn2_pressed;
lv::Style style_lbl2;

void init_styles() {
  // Button 1 — normal
  style_btn1.set(lv::prop::BgColor{}, lv::Color::from_hex(0xC0C0C0))
      .set(lv::prop::BgOpacity{}, 255)
      .set(lv::prop::Radius{}, 10)
      .set(lv::prop::BorderColor{}, lv::Color::Black())
      .set(lv::prop::BorderWidth{}, 2)
      .set(lv::prop::PadTop{}, 10)
      .set(lv::prop::PadBottom{}, 10)
      .set(lv::prop::PadLeft{}, 20)
      .set(lv::prop::PadRight{}, 20);

  // Button 1 — pressed
  style_btn1_pressed.set(lv::prop::BgColor{}, lv::Color::from_hex(0x808080));

  // Label 1 — black text
  style_lbl1.set(lv::prop::TextColor{}, lv::Color::Black());

  // Button 2 — normal (red pill)
  style_btn2.set(lv::prop::BgColor{}, lv::Color::from_hex(0xE53935))
      .set(lv::prop::BgOpacity{}, 255)
      .set(lv::prop::Radius{}, 0x7FFF)  // pill shape
      .set(lv::prop::BorderColor{}, lv::Color::from_hex(0xB71C1C))
      .set(lv::prop::BorderWidth{}, 2)
      .set(lv::prop::PadTop{}, 10)
      .set(lv::prop::PadBottom{}, 10)
      .set(lv::prop::PadLeft{}, 20)
      .set(lv::prop::PadRight{}, 20);

  // Button 2 — pressed
  style_btn2_pressed.set(lv::prop::BgColor{}, lv::Color::from_hex(0xB71C1C));

  // Label 2 — white text
  style_lbl2.set(lv::prop::TextColor{}, lv::Color::White());
}

}  // namespace

void create_ui(lv::Display& display) {
  init_styles();

  auto& scr = display.active_screen();

  // Button 1 — gray rounded
  auto& btn1 = scr.create<lv::Button>();
  btn1.set_size(150, 50);
  btn1.align(lv::Align::Center, -80, 0);
  btn1.add_style(style_btn1);
  btn1.add_style(style_btn1_pressed, lv::StyleSelector::Pressed);

  auto& lbl1 = btn1.create<lv::Label>();
  lbl1.add_style(style_lbl1);
  lbl1.set_text(std::string_view{"Button"});
  lbl1.align(lv::Align::Center);

  // Button 2 — red pill
  auto& btn2 = scr.create<lv::Button>();
  btn2.set_size(150, 50);
  btn2.align(lv::Align::Center, 80, 0);
  btn2.add_style(style_btn2);
  btn2.add_style(style_btn2_pressed, lv::StyleSelector::Pressed);

  auto& lbl2 = btn2.create<lv::Label>();
  lbl2.add_style(style_lbl2);
  lbl2.set_text(std::string_view{"Button 2"});
  lbl2.align(lv::Align::Center);
}

#if HAS_SDL2

int main() {
  return sdl2::run("Get Started 3 — Styled Buttons", 480, 320, create_ui);
}

#else

#include <cstdio>

int main() {
  std::puts("SDL2 not available. Install libsdl2-dev and rebuild.");
  return 1;
}

#endif
