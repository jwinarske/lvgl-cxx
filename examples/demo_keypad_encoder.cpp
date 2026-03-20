// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Joel Winarske
//
// Port of lv_demo_keypad_encoder — Input Demo
// Two tabs: "Selectors" with interactive widgets, "Text input" with
// keyboard-driven text areas. Click widgets to interact, type into
// text areas when focused.

#include "sdl2_harness.hpp"

#include "lvgl/widgets/container.hpp"
#include "lvgl/widgets/label.hpp"

#if HAS_SDL2

namespace {

constexpr int32_t W = 480;
constexpr int32_t H = 320;

// ── Styles ──────────────────────────────────────────────────────────────────

lv::Style scr_bg, card_bg, tab_btn, tab_btn_active;
lv::Style heading, body, value_text;
lv::Style track_bg, track_fill;
lv::Style sw_on, sw_off, sw_knob;
lv::Style cb_box, cb_check;
lv::Style dd_bg, ta_bg, ta_focus;

void init_styles() {
  scr_bg.set(lv::prop::BgColor{}, lv::Color::from_hex(0xECEFF1));
  card_bg.set(lv::prop::BgColor{}, lv::Color::White())
      .set(lv::prop::Radius{}, 10);
  tab_btn.set(lv::prop::BgColor{}, lv::Color::from_hex(0xE0E0E0))
      .set(lv::prop::Radius{}, 6)
      .set(lv::prop::TextColor{}, lv::Color::from_hex(0x757575));
  tab_btn_active.set(lv::prop::BgColor{}, lv::Color::from_hex(0x1976D2))
      .set(lv::prop::Radius{}, 6)
      .set(lv::prop::TextColor{}, lv::Color::White());
  heading.set(lv::prop::TextColor{}, lv::Color::from_hex(0x212121));
  body.set(lv::prop::TextColor{}, lv::Color::from_hex(0x616161));
  value_text.set(lv::prop::TextColor{}, lv::Color::from_hex(0x1976D2));
  track_bg.set(lv::prop::BgColor{}, lv::Color::from_hex(0xBBDEFB))
      .set(lv::prop::Radius{}, 5);
  track_fill.set(lv::prop::BgColor{}, lv::Color::from_hex(0x1976D2))
      .set(lv::prop::Radius{}, 5);
  sw_on.set(lv::prop::BgColor{}, lv::Color::from_hex(0x4CAF50))
      .set(lv::prop::Radius{}, 12);
  sw_off.set(lv::prop::BgColor{}, lv::Color::from_hex(0xBDBDBD))
      .set(lv::prop::Radius{}, 12);
  sw_knob.set(lv::prop::BgColor{}, lv::Color::White())
      .set(lv::prop::Radius{}, 100);
  cb_box.set(lv::prop::BgColor{}, lv::Color::from_hex(0xE0E0E0))
      .set(lv::prop::Radius{}, 4)
      .set(lv::prop::BorderColor{}, lv::Color::from_hex(0x9E9E9E))
      .set(lv::prop::BorderWidth{}, 2);
  cb_check.set(lv::prop::BgColor{}, lv::Color::from_hex(0x1976D2))
      .set(lv::prop::Radius{}, 4);
  dd_bg.set(lv::prop::BgColor{}, lv::Color::from_hex(0xF5F5F5))
      .set(lv::prop::Radius{}, 6)
      .set(lv::prop::BorderColor{}, lv::Color::from_hex(0xBDBDBD))
      .set(lv::prop::BorderWidth{}, 1);
  ta_bg.set(lv::prop::BgColor{}, lv::Color::from_hex(0xFAFAFA))
      .set(lv::prop::Radius{}, 6)
      .set(lv::prop::BorderColor{}, lv::Color::from_hex(0xBDBDBD))
      .set(lv::prop::BorderWidth{}, 1);
  ta_focus.set(lv::prop::BgColor{}, lv::Color::White())
      .set(lv::prop::Radius{}, 6)
      .set(lv::prop::BorderColor{}, lv::Color::from_hex(0x1976D2))
      .set(lv::prop::BorderWidth{}, 2);
}

// ── State ───────────────────────────────────────────────────────────────────

struct Rect {
  int32_t x, y, w, h;
  [[nodiscard]] bool hit(int32_t mx, int32_t my) const {
    return mx >= x && mx < x + w && my >= y && my < y + h;
  }
};

struct State {
  int tab = 0;
  bool notif_checked = false;
  int32_t brightness = 50;
  bool dark_mode = false;
  int lang_sel = 0;
  int month_sel = 2;
  int32_t quantity = 42;

  // Text input
  std::string username;
  std::string password;
  int focused_ta = -1;  // -1=none, 0=username, 1=password
};

// Tab buttons
constexpr int32_t CX = 10, CY = 55, CW = 460, CH = 250;
constexpr Rect tab_btns[] = {{18, 12, 210, 32}, {238, 12, 210, 32}};

// Selectors tab hit areas (relative to content origin CX, CY)
constexpr Rect cb_rect = {CX + 5, CY + 10, 200, 24};
constexpr Rect sl_rect = {CX + 120, CY + 50, 200, 20};
constexpr Rect sw_rect = {CX + 120, CY + 87, 50, 24};
constexpr Rect dd_rect = {CX + 120, CY + 120, 200, 30};
constexpr Rect spin_up = {CX + 215, CY + 195, 30, 24};
constexpr Rect spin_dn = {CX + 120, CY + 195, 30, 24};

// Text input hit areas
constexpr Rect ta1_rect = {CX + 10, CY + 35, 300, 30};
constexpr Rect ta2_rect = {CX + 10, CY + 105, 300, 30};

static constexpr std::string_view langs[] = {"English", "Deutsch", "Francais",
                                              "Espanol"};
static constexpr std::string_view months[] = {"Jan", "Feb", "Mar", "Apr",
                                               "May", "Jun", "Jul", "Aug",
                                               "Sep", "Oct", "Nov", "Dec"};

// ── Build ───────────────────────────────────────────────────────────────────

void build_selectors(lv::Object& c, const State& s) {
  int32_t y = 10;

  // Checkbox
  auto& cb_box_w = c.create<lv::Container>();
  cb_box_w.set_size(20, 20);
  cb_box_w.set_pos(5, y + 2);
  cb_box_w.add_style(s.notif_checked ? cb_check : cb_box);
  if (s.notif_checked) {
    auto& chk = cb_box_w.create<lv::Label>();
    chk.add_style(tab_btn_active);  // white text
    chk.set_text(std::string_view{"v"});
    chk.set_pos(4, 0);
  }
  auto& cb_lbl = c.create<lv::Label>();
  cb_lbl.add_style(body);
  cb_lbl.set_text(std::string_view{"Enable notifications"});
  cb_lbl.set_pos(32, y);
  y += 38;

  // Brightness slider
  auto& sl_lbl = c.create<lv::Label>();
  sl_lbl.add_style(body);
  sl_lbl.set_text(std::string_view{"Brightness"});
  sl_lbl.set_pos(5, y);

  auto& sl_trk = c.create<lv::Container>();
  sl_trk.set_size(200, 10);
  sl_trk.set_pos(120, y + 5);
  sl_trk.add_style(track_bg);

  auto& sl_fill = sl_trk.create<lv::Container>();
  sl_fill.set_size(s.brightness * 200 / 100, 10);
  sl_fill.set_pos(0, 0);
  sl_fill.add_style(track_fill);

  auto& sl_val = c.create<lv::Label>();
  sl_val.add_style(value_text);
  sl_val.set_text(std::to_string(s.brightness));
  sl_val.set_pos(330, y);
  y += 35;

  // Dark mode switch
  auto& sw_lbl = c.create<lv::Label>();
  sw_lbl.add_style(body);
  sw_lbl.set_text(std::string_view{"Dark mode"});
  sw_lbl.set_pos(5, y);

  auto& sw_trk = c.create<lv::Container>();
  sw_trk.set_size(50, 24);
  sw_trk.set_pos(120, y - 2);
  sw_trk.add_style(s.dark_mode ? sw_on : sw_off);

  auto& sw_k = sw_trk.create<lv::Container>();
  sw_k.set_size(20, 20);
  sw_k.set_pos(s.dark_mode ? 28 : 2, 2);
  sw_k.add_style(sw_knob);
  y += 38;

  // Language dropdown (static display)
  auto& dd_lbl = c.create<lv::Label>();
  dd_lbl.add_style(body);
  dd_lbl.set_text(std::string_view{"Language"});
  dd_lbl.set_pos(5, y);

  auto& dd_box = c.create<lv::Container>();
  dd_box.set_size(200, 28);
  dd_box.set_pos(120, y - 2);
  dd_box.add_style(dd_bg);

  auto& dd_val = dd_box.create<lv::Label>();
  dd_val.add_style(body);
  dd_val.set_text(langs[static_cast<std::size_t>(s.lang_sel)]);
  dd_val.set_pos(8, 4);

  // Down arrow
  auto& dd_arr = dd_box.create<lv::Label>();
  dd_arr.add_style(body);
  dd_arr.set_text(std::string_view{"v"});
  dd_arr.set_pos(180, 4);
  y += 40;

  // Month (static)
  auto& mo_lbl = c.create<lv::Label>();
  mo_lbl.add_style(body);
  mo_lbl.set_text(std::string_view{"Month"});
  mo_lbl.set_pos(5, y);

  auto& mo_val = c.create<lv::Label>();
  mo_val.add_style(value_text);
  mo_val.set_text(months[static_cast<std::size_t>(s.month_sel)]);
  mo_val.set_pos(120, y);
  y += 35;

  // Quantity spinbox
  auto& q_lbl = c.create<lv::Label>();
  q_lbl.add_style(body);
  q_lbl.set_text(std::string_view{"Quantity"});
  q_lbl.set_pos(5, y);

  // Minus button
  auto& minus = c.create<lv::Container>();
  minus.set_size(28, 24);
  minus.set_pos(120, y - 2);
  minus.add_style(dd_bg);
  auto& minus_lbl = minus.create<lv::Label>();
  minus_lbl.add_style(body);
  minus_lbl.set_text(std::string_view{"-"});
  minus_lbl.set_pos(9, 2);

  // Value
  auto& q_val = c.create<lv::Label>();
  q_val.add_style(value_text);
  q_val.set_text(std::to_string(s.quantity));
  q_val.set_pos(165, y);

  // Plus button
  auto& plus = c.create<lv::Container>();
  plus.set_size(28, 24);
  plus.set_pos(215, y - 2);
  plus.add_style(dd_bg);
  auto& plus_lbl = plus.create<lv::Label>();
  plus_lbl.add_style(body);
  plus_lbl.set_text(std::string_view{"+"});
  plus_lbl.set_pos(7, 2);
}

void build_text_input(lv::Object& c, const State& s) {
  auto& u_lbl = c.create<lv::Label>();
  u_lbl.add_style(heading);
  u_lbl.set_text(std::string_view{"Username"});
  u_lbl.set_pos(10, 10);

  auto& ta1 = c.create<lv::Container>();
  ta1.set_size(300, 28);
  ta1.set_pos(10, 35);
  ta1.add_style(s.focused_ta == 0 ? ta_focus : ta_bg);

  auto& ta1_txt = ta1.create<lv::Label>();
  ta1_txt.add_style(s.username.empty() ? body : heading);
  ta1_txt.set_text(s.username.empty() ? std::string_view{"Enter username"}
                                      : std::string_view{s.username});
  ta1_txt.set_pos(8, 4);

  // Cursor indicator when focused
  if (s.focused_ta == 0) {
    int32_t cursor_x =
        8 + static_cast<int32_t>(s.username.size()) * 8;  // approximate
    auto& cursor = ta1.create<lv::Container>();
    static lv::Style cursor_style;
    cursor_style.set(lv::prop::BgColor{}, lv::Color::from_hex(0x1976D2));
    cursor.set_size(2, 16);
    cursor.set_pos(cursor_x, 5);
    cursor.add_style(cursor_style);
  }

  auto& p_lbl = c.create<lv::Label>();
  p_lbl.add_style(heading);
  p_lbl.set_text(std::string_view{"Password"});
  p_lbl.set_pos(10, 78);

  auto& ta2 = c.create<lv::Container>();
  ta2.set_size(300, 28);
  ta2.set_pos(10, 105);
  ta2.add_style(s.focused_ta == 1 ? ta_focus : ta_bg);

  // Show dots for password
  std::string display_pw(s.password.size(), '*');
  auto& ta2_txt = ta2.create<lv::Label>();
  ta2_txt.add_style(s.password.empty() ? body : heading);
  ta2_txt.set_text(s.password.empty() ? std::string_view{"Enter password"}
                                      : std::string_view{display_pw});
  ta2_txt.set_pos(8, 4);

  if (s.focused_ta == 1) {
    int32_t cursor_x = 8 + static_cast<int32_t>(s.password.size()) * 8;
    auto& cursor = ta2.create<lv::Container>();
    static lv::Style cursor_style;
    cursor_style.set(lv::prop::BgColor{}, lv::Color::from_hex(0x1976D2));
    cursor.set_size(2, 16);
    cursor.set_pos(cursor_x, 5);
    cursor.add_style(cursor_style);
  }

  // Hint
  auto& hint = c.create<lv::Label>();
  hint.add_style(body);
  hint.set_text(std::string_view{"Click a field, then type. Backspace to delete."});
  hint.set_pos(10, 150);
}

void rebuild(lv::Object& scr, const State& s) {
  scr.remove_all_children();
  scr.remove_all_styles();
  scr.add_style(scr_bg);

  // Tab bar
  auto& tbar = scr.create<lv::Container>();
  tbar.set_size(460, 40);
  tbar.set_pos(10, 8);

  static constexpr std::string_view tnames[] = {"Selectors", "Text input"};
  for (int i = 0; i < 2; ++i) {
    auto& btn = tbar.create<lv::Container>();
    btn.set_size(210, 32);
    btn.set_pos(8 + i * 220, 4);
    btn.add_style(i == s.tab ? tab_btn_active : tab_btn);

    auto& lbl = btn.create<lv::Label>();
    lbl.add_style(i == s.tab ? tab_btn_active : tab_btn);
    lbl.set_text(tnames[i]);
    lbl.align(lv::Align::Center);
  }

  // Content card
  auto& content = scr.create<lv::Container>();
  content.set_size(CW, CH);
  content.set_pos(CX, CY);
  content.add_style(card_bg);

  if (s.tab == 0)
    build_selectors(content, s);
  else
    build_text_input(content, s);
}

}  // namespace

int main() {
  sdl2::init(W, H, "Demo Keypad & Encoder");
  init_styles();

  std::vector<lv::Display::Pixel> buf(
      static_cast<std::size_t>(W) * static_cast<std::size_t>(H));
  lv::Display display{W, H, sdl2::flush_cb};
  display.set_draw_buffers(buf);
  display.set_render_mode(lv::RenderMode::Full);

  State state;
  auto& scr = display.active_screen();
  rebuild(scr, state);
  display.refresh();

  bool dirty = false;
  lv::Ticker ticker;

  while (sdl2::ctx.running) {
    dirty = false;

    // Poll SDL events — handle keyboard separately for text input
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
      switch (ev.type) {
        case SDL_QUIT:
          sdl2::ctx.running = false;
          break;
        case SDL_MOUSEMOTION:
          sdl2::ctx.mouse_x = ev.motion.x;
          sdl2::ctx.mouse_y = ev.motion.y;
          break;
        case SDL_MOUSEBUTTONDOWN: {
          sdl2::ctx.mouse_pressed = true;
          const int32_t mx = ev.button.x;
          const int32_t my = ev.button.y;

          // Tab switching
          for (int i = 0; i < 2; ++i) {
            if (tab_btns[i].hit(mx, my) && state.tab != i) {
              state.tab = i;
              state.focused_ta = -1;
              dirty = true;
            }
          }

          if (state.tab == 0) {
            // Checkbox
            if (cb_rect.hit(mx, my)) {
              state.notif_checked = !state.notif_checked;
              dirty = true;
            }
            // Switch
            if (sw_rect.hit(mx, my)) {
              state.dark_mode = !state.dark_mode;
              dirty = true;
            }
            // Dropdown — cycle language
            if (dd_rect.hit(mx, my)) {
              state.lang_sel = (state.lang_sel + 1) % 4;
              dirty = true;
            }
            // Spinbox minus
            if (spin_dn.hit(mx, my) && state.quantity > 0) {
              --state.quantity;
              dirty = true;
            }
            // Spinbox plus
            if (spin_up.hit(mx, my) && state.quantity < 999) {
              ++state.quantity;
              dirty = true;
            }
          }

          if (state.tab == 1) {
            // Focus text areas
            if (ta1_rect.hit(mx, my)) {
              state.focused_ta = 0;
              dirty = true;
            } else if (ta2_rect.hit(mx, my)) {
              state.focused_ta = 1;
              dirty = true;
            } else {
              if (state.focused_ta >= 0) {
                state.focused_ta = -1;
                dirty = true;
              }
            }
          }
          break;
        }
        case SDL_MOUSEBUTTONUP:
          sdl2::ctx.mouse_pressed = false;
          break;

        case SDL_KEYDOWN: {
          // Keyboard input for text areas
          if (state.tab == 1 && state.focused_ta >= 0) {
            auto& target =
                state.focused_ta == 0 ? state.username : state.password;
            if (ev.key.keysym.sym == SDLK_BACKSPACE) {
              if (!target.empty()) {
                target.pop_back();
                dirty = true;
              }
            } else if (ev.key.keysym.sym == SDLK_RETURN) {
              // Move to next field or unfocus
              if (state.focused_ta == 0)
                state.focused_ta = 1;
              else
                state.focused_ta = -1;
              dirty = true;
            } else if (ev.key.keysym.sym == SDLK_TAB) {
              state.focused_ta = 1 - state.focused_ta;
              dirty = true;
            }
          }
          break;
        }
        case SDL_TEXTINPUT: {
          if (state.tab == 1 && state.focused_ta >= 0) {
            auto& target =
                state.focused_ta == 0 ? state.username : state.password;
            target += ev.text.text;
            dirty = true;
          }
          break;
        }
        default:
          break;
      }
    }

    // Slider drag
    if (state.tab == 0 && sdl2::ctx.mouse_pressed) {
      const int32_t mx = sdl2::ctx.mouse_x;
      const int32_t my = sdl2::ctx.mouse_y;
      if (mx >= sl_rect.x && mx <= sl_rect.x + sl_rect.w &&
          my >= sl_rect.y - 10 && my <= sl_rect.y + sl_rect.h + 10) {
        int32_t val = (mx - sl_rect.x) * 100 / sl_rect.w;
        val = std::clamp(val, int32_t{0}, int32_t{100});
        if (val != state.brightness) {
          state.brightness = val;
          dirty = true;
        }
      }
    }

    if (dirty)
      rebuild(scr, state);

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
