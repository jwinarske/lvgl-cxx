// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Joel Winarske
//
// Port of lv_demo_widgets — Widget Showcase
// Interactive TabView with Profile, Analytics, and Shop tabs.
// Click tabs to switch, click switch to toggle, drag slider.

#include "sdl2_harness.hpp"

#include "lvgl/widgets/container.hpp"
#include "lvgl/widgets/label.hpp"

#if HAS_SDL2

namespace {

constexpr int32_t W = 480;
constexpr int32_t H = 320;

// ── Styles ──────────────────────────────────────────────────────────────────

lv::Style scr_style;
lv::Style heading_style;
lv::Style body_text;
lv::Style card_style;
lv::Style tab_btn_style;
lv::Style tab_btn_active;
lv::Style slider_track;
lv::Style slider_indicator;
lv::Style bar_track;
lv::Style bar_fill;
lv::Style switch_on;
lv::Style switch_off;
lv::Style knob_style;
lv::Style list_bg;
lv::Style list_btn_style;
lv::Style led_on;
lv::Style led_off;
lv::Style chart_bg;
lv::Style blue_bar;
lv::Style green_bar;
lv::Style price_style;

void init_styles() {
  scr_style.set(lv::prop::BgColor{}, lv::Color::from_hex(0xECEFF1));
  heading_style.set(lv::prop::TextColor{}, lv::Color::from_hex(0x212121));
  body_text.set(lv::prop::TextColor{}, lv::Color::from_hex(0x616161));
  card_style.set(lv::prop::BgColor{}, lv::Color::White())
      .set(lv::prop::Radius{}, 12);
  tab_btn_style.set(lv::prop::BgColor{}, lv::Color::from_hex(0xE0E0E0))
      .set(lv::prop::Radius{}, 6)
      .set(lv::prop::TextColor{}, lv::Color::from_hex(0x757575));
  tab_btn_active.set(lv::prop::BgColor{}, lv::Color::from_hex(0x1976D2))
      .set(lv::prop::Radius{}, 6)
      .set(lv::prop::TextColor{}, lv::Color::White());
  slider_track.set(lv::prop::BgColor{}, lv::Color::from_hex(0xBBDEFB))
      .set(lv::prop::Radius{}, 5);
  slider_indicator.set(lv::prop::BgColor{}, lv::Color::from_hex(0x1976D2))
      .set(lv::prop::Radius{}, 5);
  bar_track.set(lv::prop::BgColor{}, lv::Color::from_hex(0xC8E6C9))
      .set(lv::prop::Radius{}, 5);
  bar_fill.set(lv::prop::BgColor{}, lv::Color::from_hex(0x388E3C))
      .set(lv::prop::Radius{}, 5);
  switch_on.set(lv::prop::BgColor{}, lv::Color::from_hex(0x4CAF50))
      .set(lv::prop::Radius{}, 12);
  switch_off.set(lv::prop::BgColor{}, lv::Color::from_hex(0xBDBDBD))
      .set(lv::prop::Radius{}, 12);
  knob_style.set(lv::prop::BgColor{}, lv::Color::White())
      .set(lv::prop::Radius{}, 100);
  list_bg.set(lv::prop::BgColor{}, lv::Color::White())
      .set(lv::prop::Radius{}, 8);
  list_btn_style.set(lv::prop::BgColor{}, lv::Color::from_hex(0xF5F5F5))
      .set(lv::prop::Radius{}, 4)
      .set(lv::prop::TextColor{}, lv::Color::from_hex(0x424242));
  led_on.set(lv::prop::BgColor{}, lv::Color::from_hex(0x4CAF50))
      .set(lv::prop::Radius{}, 100);
  led_off.set(lv::prop::BgColor{}, lv::Color::from_hex(0xBDBDBD))
      .set(lv::prop::Radius{}, 100);
  chart_bg.set(lv::prop::BgColor{}, lv::Color::White())
      .set(lv::prop::Radius{}, 8);
  blue_bar.set(lv::prop::BgColor{}, lv::Color::from_hex(0x2196F3))
      .set(lv::prop::Radius{}, 2);
  green_bar.set(lv::prop::BgColor{}, lv::Color::from_hex(0x4CAF50))
      .set(lv::prop::Radius{}, 2);
  price_style.set(lv::prop::TextColor{}, lv::Color::from_hex(0x1976D2));
}

// ── Layout constants ────────────────────────────────────────────────────────

constexpr int32_t TAB_BAR_X = 10;
constexpr int32_t TAB_BAR_Y = 8;
constexpr int32_t TAB_BTN_W = 140;
constexpr int32_t TAB_BTN_H = 32;
constexpr int32_t TAB_BTN_GAP = 150;
constexpr int32_t CONTENT_X = 10;
constexpr int32_t CONTENT_Y = 55;
constexpr int32_t CONTENT_W = 460;
constexpr int32_t CONTENT_H = 250;

// ── Interactive state ───────────────────────────────────────────────────────

struct UIState {
  int active_tab = 0;
  bool notif_on = true;
  int32_t volume = 65;

  // Absolute rects for hit-testing
  struct Rect {
    int32_t x, y, w, h;
    [[nodiscard]] bool contains(const int32_t mx, const int32_t my) const {
      return mx >= x && mx < x + w && my >= y && my < y + h;
    }
  };

  Rect tab_btns[3] = {
      {TAB_BAR_X + 8 + 0 * TAB_BTN_GAP, TAB_BAR_Y + 4, TAB_BTN_W, TAB_BTN_H},
      {TAB_BAR_X + 8 + 1 * TAB_BTN_GAP, TAB_BAR_Y + 4, TAB_BTN_W, TAB_BTN_H},
      {TAB_BAR_X + 8 + 2 * TAB_BTN_GAP, TAB_BAR_Y + 4, TAB_BTN_W, TAB_BTN_H},
  };
  Rect switch_rect = {CONTENT_X + 170, CONTENT_Y + 42, 50, 24};
  Rect slider_rect = {CONTENT_X + 110, CONTENT_Y + 85, 200, 20};
};

// ── Build functions ─────────────────────────────────────────────────────────

void build_tab_bar(lv::Object& scr, const UIState& st) {
  auto& bar = scr.create<lv::Container>();
  bar.set_size(460, 40);
  bar.set_pos(TAB_BAR_X, TAB_BAR_Y);

  static constexpr std::string_view names[] = {"Profile", "Analytics", "Shop"};
  for (int i = 0; i < 3; ++i) {
    auto& btn = bar.create<lv::Container>();
    btn.set_size(TAB_BTN_W, TAB_BTN_H);
    btn.set_pos(8 + i * TAB_BTN_GAP, 4);
    btn.add_style(i == st.active_tab ? tab_btn_active : tab_btn_style);

    auto& lbl = btn.create<lv::Label>();
    lbl.add_style(i == st.active_tab ? tab_btn_active : tab_btn_style);
    lbl.set_text(names[i]);
    lbl.align(lv::Align::Center);
  }
}

void build_profile(lv::Object& content, const UIState& st) {
  auto& name = content.create<lv::Label>();
  name.add_style(heading_style);
  name.set_text(std::string_view{"John Smith"});
  name.set_pos(15, 10);

  // Notifications switch
  auto& notif_lbl = content.create<lv::Label>();
  notif_lbl.add_style(body_text);
  notif_lbl.set_text(std::string_view{"Notifications"});
  notif_lbl.set_pos(15, 45);

  auto& sw_track = content.create<lv::Container>();
  sw_track.set_size(50, 24);
  sw_track.set_pos(170, 42);
  sw_track.add_style(st.notif_on ? switch_on : switch_off);

  auto& sw_knob = sw_track.create<lv::Container>();
  sw_knob.set_size(20, 20);
  sw_knob.set_pos(st.notif_on ? 28 : 2, 2);
  sw_knob.add_style(knob_style);

  // Volume slider
  auto& vol_lbl = content.create<lv::Label>();
  vol_lbl.add_style(body_text);
  vol_lbl.set_text(std::string_view{"Volume"});
  vol_lbl.set_pos(15, 88);

  auto& sl_bg = content.create<lv::Container>();
  sl_bg.set_size(200, 10);
  sl_bg.set_pos(110, 90);
  sl_bg.add_style(slider_track);

  auto& sl_fill = sl_bg.create<lv::Container>();
  sl_fill.set_size(st.volume * 200 / 100, 10);
  sl_fill.set_pos(0, 0);
  sl_fill.add_style(slider_indicator);

  auto& vol_val = content.create<lv::Label>();
  vol_val.add_style(body_text);
  vol_val.set_text(std::to_string(st.volume));
  vol_val.set_pos(320, 85);

  // Progress bar
  auto& prog_lbl = content.create<lv::Label>();
  prog_lbl.add_style(body_text);
  prog_lbl.set_text(std::string_view{"Progress"});
  prog_lbl.set_pos(15, 125);

  auto& bar_bg = content.create<lv::Container>();
  bar_bg.set_size(200, 14);
  bar_bg.set_pos(110, 127);
  bar_bg.add_style(bar_track);

  auto& bar_f = bar_bg.create<lv::Container>();
  bar_f.set_size(140, 14);
  bar_f.set_pos(0, 0);
  bar_f.add_style(bar_fill);

  // LED status
  auto& stat_lbl = content.create<lv::Label>();
  stat_lbl.add_style(body_text);
  stat_lbl.set_text(std::string_view{"Status"});
  stat_lbl.set_pos(15, 162);

  auto& led = content.create<lv::Container>();
  led.set_size(16, 16);
  led.set_pos(110, 162);
  led.add_style(st.notif_on ? led_on : led_off);

  auto& stat_val = content.create<lv::Label>();
  stat_val.add_style(body_text);
  stat_val.set_text(st.notif_on ? std::string_view{"Online"}
                                : std::string_view{"Offline"});
  stat_val.set_pos(135, 162);
}

void build_analytics(lv::Object& content) {
  auto& card = content.create<lv::Container>();
  card.set_size(430, 140);
  card.set_pos(10, 10);
  card.add_style(chart_bg);

  auto& title = card.create<lv::Label>();
  title.add_style(heading_style);
  title.set_text(std::string_view{"Revenue & Users"});
  title.set_pos(10, 5);

  for (int i = 0; i < 10; ++i) {
    constexpr int32_t usr[] = {20, 30, 25, 40, 55, 38, 60, 50, 70, 45};
    constexpr int32_t rev[] = {34, 52, 47, 61, 78, 55, 83, 72, 90, 68};
    const int32_t h1 = rev[i] * 80 / 100;
    auto& b1 = card.create<lv::Container>();
    b1.set_size(14, h1);
    b1.set_pos(15 + i * 40, 120 - h1);
    b1.add_style(blue_bar);

    const int32_t h2 = usr[i] * 80 / 100;
    auto& b2 = card.create<lv::Container>();
    b2.set_size(14, h2);
    b2.set_pos(15 + i * 40 + 16, 120 - h2);
    b2.add_style(green_bar);
  }

  auto& legend = content.create<lv::Label>();
  legend.add_style(body_text);
  legend.set_text(std::string_view{"Blue: Revenue  |  Green: Users"});
  legend.set_pos(10, 160);
}

void build_shop(lv::Object& content) {
  auto& list = content.create<lv::Container>();
  list.set_size(430, 210);
  list.set_pos(10, 5);
  list.add_style(list_bg);

  auto& header = list.create<lv::Label>();
  header.add_style(heading_style);
  header.set_text(std::string_view{"Catalog"});
  header.set_pos(10, 5);

  struct Item {
    std::string_view name;
    std::string_view price;
  };

  for (int i = 0; i < 6; ++i) {
    constexpr Item items[] = {
        {"T-shirt", "$20"}, {"Shoes", "$50"}, {"Watch", "$100"},
        {"Bag", "$35"},     {"Hat", "$15"},   {"Jacket", "$80"},
    };
    auto& row = list.create<lv::Container>();
    row.set_size(410, 26);
    row.set_pos(10, 30 + i * 29);
    row.add_style(list_btn_style);

    auto& nm = row.create<lv::Label>();
    nm.add_style(body_text);
    nm.set_text(items[i].name);
    nm.set_pos(8, 3);

    auto& pr = row.create<lv::Label>();
    pr.add_style(price_style);
    pr.set_text(items[i].price);
    pr.set_pos(350, 3);
  }
}

void rebuild_ui(lv::Object& scr, const UIState& st) {
  scr.remove_all_children();
  scr.remove_all_styles();
  scr.add_style(scr_style);

  build_tab_bar(scr, st);

  auto& content = scr.create<lv::Container>();
  content.set_size(CONTENT_W, CONTENT_H);
  content.set_pos(CONTENT_X, CONTENT_Y);
  content.add_style(card_style);

  switch (st.active_tab) {
    case 0:
      build_profile(content, st);
      break;
    case 1:
      build_analytics(content);
      break;
    case 2:
      build_shop(content);
      break;
    default:
      break;
  }
}

}  // namespace

int main() {
  sdl2::init(W, H, "Demo Widgets");
  init_styles();

  std::vector<lv::Display::Pixel> buf(
      static_cast<std::size_t>(W) * static_cast<std::size_t>(H));
  lv::Display display{W, H, sdl2::flush_cb};
  display.set_draw_buffers(buf);
  display.set_render_mode(lv::RenderMode::Full);

  UIState state;
  auto& scr = display.active_screen();
  rebuild_ui(scr, state);
  display.refresh();

  bool needs_rebuild = false;
  lv::Ticker ticker;

  while (sdl2::ctx.running) {
    sdl2::poll_events();
    ticker.update();

    if (sdl2::ctx.mouse_clicked) {
      const int32_t mx = sdl2::ctx.mouse_x;
      const int32_t my = sdl2::ctx.mouse_y;

      // Tab buttons
      for (int i = 0; i < 3; ++i) {
        if (state.tab_btns[i].contains(mx, my) && state.active_tab != i) {
          state.active_tab = i;
          needs_rebuild = true;
        }
      }

      // Switch toggle (only on the Profile tab)
      if (state.active_tab == 0 && state.switch_rect.contains(mx, my)) {
        state.notif_on = !state.notif_on;
        needs_rebuild = true;
      }
    }

    // Slider drag (only on the Profile tab)
    if (state.active_tab == 0 && sdl2::ctx.mouse_pressed) {
      const int32_t mx = sdl2::ctx.mouse_x;
      const int32_t my = sdl2::ctx.mouse_y;
      auto& [x, y, w, h] = state.slider_rect;
      if (mx >= x && mx <= x + w &&
          my >= y - 10 && my <= y + h + 10) {
        int32_t val = (mx - x) * 100 / w;
        val = std::clamp(val, int32_t{0}, int32_t{100});
        if (val != state.volume) {
          state.volume = val;
          needs_rebuild = true;
        }
      }
    }

    if (needs_rebuild) {
      rebuild_ui(scr, state);
      needs_rebuild = false;
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
