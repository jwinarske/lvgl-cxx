// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Joel Winarske
//
// Port of lv_demo_stress — Stress Test
// Creates and destroys batches of styled widgets each cycle to exercise
// the object tree, styles, and rendering pipeline under churn.

#include "sdl2_harness.hpp"

#include "lvgl/widgets/arc.hpp"
#include "lvgl/widgets/container.hpp"
#include "lvgl/widgets/label.hpp"

#if HAS_SDL2

namespace {

constexpr int32_t DW = 480;
constexpr int32_t DH = 320;
constexpr int FRAMES_PER_CYCLE = 60;

// ── Styles ──────────────────────────────────────────────────────────────────

lv::Style scr_bg;
lv::Style btn_style, btn_alt;
lv::Style card_style;
lv::Style bar_track, bar_fill;
lv::Style sw_on, sw_off, sw_knob;
lv::Style chip_styles[6];
lv::Style tab_active, tab_inactive;
lv::Style heading, body_text;
lv::Style input_style;
lv::Style grid_cell;
lv::Style list_row;

constexpr uint32_t palette[] = {0x2196F3, 0x4CAF50, 0xFF9800,
                                 0xE91E63, 0x9C27B0, 0x00BCD4};

void init_styles() {
  scr_bg.set(lv::prop::BgColor{}, lv::Color::from_hex(0xF5F5F5));
  btn_style.set(lv::prop::BgColor{}, lv::Color::from_hex(0x1976D2))
      .set(lv::prop::Radius{}, 8);
  btn_alt.set(lv::prop::BgColor{}, lv::Color::from_hex(0x388E3C))
      .set(lv::prop::Radius{}, 8);
  card_style.set(lv::prop::BgColor{}, lv::Color::White())
      .set(lv::prop::Radius{}, 10);
  bar_track.set(lv::prop::BgColor{}, lv::Color::from_hex(0xE0E0E0))
      .set(lv::prop::Radius{}, 5);
  bar_fill.set(lv::prop::BgColor{}, lv::Color::from_hex(0x1976D2))
      .set(lv::prop::Radius{}, 5);
  sw_on.set(lv::prop::BgColor{}, lv::Color::from_hex(0x4CAF50))
      .set(lv::prop::Radius{}, 12);
  sw_off.set(lv::prop::BgColor{}, lv::Color::from_hex(0xBDBDBD))
      .set(lv::prop::Radius{}, 12);
  sw_knob.set(lv::prop::BgColor{}, lv::Color::White())
      .set(lv::prop::Radius{}, 100);
  for (int i = 0; i < 6; ++i)
    chip_styles[i]
        .set(lv::prop::BgColor{}, lv::Color::from_hex(palette[i]))
        .set(lv::prop::Radius{}, 4);
  tab_active.set(lv::prop::BgColor{}, lv::Color::from_hex(0x1976D2))
      .set(lv::prop::Radius{}, 6)
      .set(lv::prop::TextColor{}, lv::Color::White());
  tab_inactive.set(lv::prop::BgColor{}, lv::Color::from_hex(0xE0E0E0))
      .set(lv::prop::Radius{}, 6)
      .set(lv::prop::TextColor{}, lv::Color::from_hex(0x757575));
  heading.set(lv::prop::TextColor{}, lv::Color::from_hex(0x212121));
  body_text.set(lv::prop::TextColor{}, lv::Color::from_hex(0x616161));
  input_style.set(lv::prop::BgColor{}, lv::Color::from_hex(0xFAFAFA))
      .set(lv::prop::Radius{}, 6)
      .set(lv::prop::BorderColor{}, lv::Color::from_hex(0xBDBDBD))
      .set(lv::prop::BorderWidth{}, 1);
  grid_cell.set(lv::prop::BgColor{}, lv::Color::from_hex(0xE3F2FD))
      .set(lv::prop::Radius{}, 4);
  list_row.set(lv::prop::BgColor{}, lv::Color::from_hex(0xF5F5F5))
      .set(lv::prop::Radius{}, 4);
}

// ── Batch builders ──────────────────────────────────────────────────────────

void batch_buttons(lv::Object& p) {
  auto& title = p.create<lv::Label>();
  title.add_style(heading);
  title.set_text(std::string_view{"Buttons"});
  title.set_pos(10, 5);

  for (int i = 0; i < 8; ++i) {
    auto& btn = p.create<lv::Container>();
    btn.set_size(100, 36);
    btn.set_pos(10 + (i % 4) * 112, 30 + (i / 4) * 46);
    btn.add_style(i % 2 == 0 ? btn_style : btn_alt);

    auto& lbl = btn.create<lv::Label>();
    lbl.add_style(tab_active);
    lbl.set_text("Btn " + std::to_string(i + 1));
    lbl.align(lv::Align::Center);
  }
}

void batch_cards(lv::Object& p) {
  auto& title = p.create<lv::Label>();
  title.add_style(heading);
  title.set_text(std::string_view{"Cards"});
  title.set_pos(10, 5);

  static constexpr std::string_view names[] = {"Alpha", "Bravo", "Charlie",
                                                "Delta", "Echo", "Foxtrot"};
  for (int i = 0; i < 6; ++i) {
    auto& card = p.create<lv::Container>();
    card.set_size(140, 80);
    card.set_pos(10 + (i % 3) * 150, 30 + (i / 3) * 90);
    card.add_style(card_style);

    auto& lbl = card.create<lv::Label>();
    lbl.add_style(heading);
    lbl.set_text(names[i]);
    lbl.align(lv::Align::Center);
  }
}

void batch_bars(lv::Object& p) {
  auto& title = p.create<lv::Label>();
  title.add_style(heading);
  title.set_text(std::string_view{"Progress Bars"});
  title.set_pos(10, 5);

  for (int i = 0; i < 6; ++i) {
    auto& lbl = p.create<lv::Label>();
    lbl.add_style(body_text);
    lbl.set_text("Task " + std::to_string(i + 1));
    lbl.set_pos(10, 35 + i * 38);

    auto& track = p.create<lv::Container>();
    track.set_size(280, 12);
    track.set_pos(100, 38 + i * 38);
    track.add_style(bar_track);

    int32_t pct = static_cast<int32_t>((i * 37 + 13) % 100);
    auto& fill = track.create<lv::Container>();
    fill.set_size(pct * 280 / 100, 12);
    fill.set_pos(0, 0);
    fill.add_style(bar_fill);

    auto& val = p.create<lv::Label>();
    val.add_style(body_text);
    val.set_text(std::to_string(pct) + "%");
    val.set_pos(395, 35 + i * 38);
  }
}

void batch_switches(lv::Object& p) {
  auto& title = p.create<lv::Label>();
  title.add_style(heading);
  title.set_text(std::string_view{"Switches & Checkboxes"});
  title.set_pos(10, 5);

  static constexpr std::string_view labels[] = {
      "WiFi", "Bluetooth", "GPS", "NFC", "Auto-sync", "Dark mode"};
  for (int i = 0; i < 6; ++i) {
    auto& lbl = p.create<lv::Label>();
    lbl.add_style(body_text);
    lbl.set_text(labels[i]);
    lbl.set_pos(10, 35 + i * 38);

    bool on = (i % 2 == 0);
    auto& track = p.create<lv::Container>();
    track.set_size(44, 22);
    track.set_pos(160, 33 + i * 38);
    track.add_style(on ? sw_on : sw_off);

    auto& knob = track.create<lv::Container>();
    knob.set_size(18, 18);
    knob.set_pos(on ? 24 : 2, 2);
    knob.add_style(sw_knob);
  }
}

void batch_inputs(lv::Object& p) {
  auto& title = p.create<lv::Label>();
  title.add_style(heading);
  title.set_text(std::string_view{"Text Inputs"});
  title.set_pos(10, 5);

  static constexpr std::string_view fields[] = {"Name", "Email", "Phone",
                                                 "Address"};
  static constexpr std::string_view values[] = {
      "John Smith", "john@example.com", "+1 555-0123", "123 Main St"};

  for (int i = 0; i < 4; ++i) {
    auto& lbl = p.create<lv::Label>();
    lbl.add_style(body_text);
    lbl.set_text(fields[i]);
    lbl.set_pos(10, 35 + i * 50);

    auto& input = p.create<lv::Container>();
    input.set_size(300, 26);
    input.set_pos(100, 33 + i * 50);
    input.add_style(input_style);

    auto& val = input.create<lv::Label>();
    val.add_style(heading);
    val.set_text(values[i]);
    val.set_pos(8, 3);
  }
}

void batch_grid(lv::Object& p) {
  auto& title = p.create<lv::Label>();
  title.add_style(heading);
  title.set_text(std::string_view{"Color Grid"});
  title.set_pos(10, 5);

  for (int r = 0; r < 4; ++r) {
    for (int c = 0; c < 8; ++c) {
      auto& cell = p.create<lv::Container>();
      cell.set_size(50, 50);
      cell.set_pos(10 + c * 56, 30 + r * 56);
      cell.add_style(chip_styles[(r * 8 + c) % 6]);

      auto& num = cell.create<lv::Label>();
      num.add_style(tab_active);
      num.set_text(std::to_string(r * 8 + c + 1));
      num.align(lv::Align::Center);
    }
  }
}

void batch_tabs(lv::Object& p) {
  auto& title = p.create<lv::Label>();
  title.add_style(heading);
  title.set_text(std::string_view{"Tab Layout"});
  title.set_pos(10, 5);

  // Tab bar
  auto& tbar = p.create<lv::Container>();
  tbar.set_size(440, 34);
  tbar.set_pos(10, 28);

  static constexpr std::string_view tabs[] = {"Home", "Search", "Settings",
                                               "Profile"};
  for (int i = 0; i < 4; ++i) {
    auto& btn = tbar.create<lv::Container>();
    btn.set_size(100, 28);
    btn.set_pos(5 + i * 108, 3);
    btn.add_style(i == 0 ? tab_active : tab_inactive);

    auto& lbl = btn.create<lv::Label>();
    lbl.add_style(i == 0 ? tab_active : tab_inactive);
    lbl.set_text(tabs[i]);
    lbl.align(lv::Align::Center);
  }

  // Content
  auto& content = p.create<lv::Container>();
  content.set_size(440, 200);
  content.set_pos(10, 68);
  content.add_style(card_style);

  auto& msg = content.create<lv::Label>();
  msg.add_style(heading);
  msg.set_text(std::string_view{"Welcome to the Home tab"});
  msg.align(lv::Align::Center);
}

void batch_list(lv::Object& p) {
  auto& title = p.create<lv::Label>();
  title.add_style(heading);
  title.set_text(std::string_view{"Item List"});
  title.set_pos(10, 5);

  auto& list = p.create<lv::Container>();
  list.set_size(440, 250);
  list.set_pos(10, 28);
  list.add_style(card_style);

  for (int i = 0; i < 8; ++i) {
    auto& row = list.create<lv::Container>();
    row.set_size(420, 26);
    row.set_pos(10, 8 + i * 30);
    row.add_style(list_row);

    auto& lbl = row.create<lv::Label>();
    lbl.add_style(body_text);
    lbl.set_text("Item " + std::to_string(i + 1));
    lbl.set_pos(8, 3);

    auto& tag = row.create<lv::Container>();
    tag.set_size(50, 18);
    tag.set_pos(360, 4);
    tag.add_style(chip_styles[i % 6]);

    auto& tag_lbl = tag.create<lv::Label>();
    tag_lbl.add_style(tab_active);
    tag_lbl.set_text(std::string_view{"tag"});
    tag_lbl.set_pos(10, 0);
  }
}

void batch_arcs(lv::Object& p) {
  auto& title = p.create<lv::Label>();
  title.add_style(heading);
  title.set_text(std::string_view{"Arc Gauges"});
  title.set_pos(10, 5);

  // Arcs are rendered by the arc renderer
  auto& arc1 = p.create<lv::Arc>();
  arc1.set_size(100, 100);
  arc1.set_pos(30, 40);
  arc1.set_range(0, 100);
  arc1.set_value(75);
  arc1.set_bg_start_angle(135);
  arc1.set_bg_end_angle(45);

  auto& arc2 = p.create<lv::Arc>();
  arc2.set_size(100, 100);
  arc2.set_pos(180, 40);
  arc2.set_range(0, 100);
  arc2.set_value(40);
  arc2.set_bg_start_angle(180);
  arc2.set_bg_end_angle(0);

  auto& arc3 = p.create<lv::Arc>();
  arc3.set_size(100, 100);
  arc3.set_pos(330, 40);
  arc3.set_range(0, 100);
  arc3.set_value(90);
  arc3.set_bg_start_angle(0);
  arc3.set_bg_end_angle(360);

  // Labels below
  auto& l1 = p.create<lv::Label>();
  l1.add_style(body_text);
  l1.set_text(std::string_view{"CPU 75%"});
  l1.set_pos(45, 150);

  auto& l2 = p.create<lv::Label>();
  l2.add_style(body_text);
  l2.set_text(std::string_view{"RAM 40%"});
  l2.set_pos(195, 150);

  auto& l3 = p.create<lv::Label>();
  l3.add_style(body_text);
  l3.set_text(std::string_view{"Disk 90%"});
  l3.set_pos(345, 150);
}

void batch_mixed(lv::Object& p) {
  auto& title = p.create<lv::Label>();
  title.add_style(heading);
  title.set_text(std::string_view{"Mixed Widgets"});
  title.set_pos(10, 5);

  // A card with a bar, switch, and buttons
  auto& card = p.create<lv::Container>();
  card.set_size(220, 200);
  card.set_pos(10, 30);
  card.add_style(card_style);

  auto& card_title = card.create<lv::Label>();
  card_title.add_style(heading);
  card_title.set_text(std::string_view{"Dashboard"});
  card_title.set_pos(10, 5);

  auto& track = card.create<lv::Container>();
  track.set_size(180, 10);
  track.set_pos(15, 35);
  track.add_style(bar_track);
  auto& fill = track.create<lv::Container>();
  fill.set_size(120, 10);
  fill.set_pos(0, 0);
  fill.add_style(bar_fill);

  auto& sw_trk = card.create<lv::Container>();
  sw_trk.set_size(44, 22);
  sw_trk.set_pos(15, 60);
  sw_trk.add_style(sw_on);
  auto& knob = sw_trk.create<lv::Container>();
  knob.set_size(18, 18);
  knob.set_pos(24, 2);
  knob.add_style(sw_knob);

  auto& sw_lbl = card.create<lv::Label>();
  sw_lbl.add_style(body_text);
  sw_lbl.set_text(std::string_view{"Active"});
  sw_lbl.set_pos(70, 62);

  // Side panel
  auto& side = p.create<lv::Container>();
  side.set_size(220, 200);
  side.set_pos(245, 30);
  side.add_style(card_style);

  auto& side_title = side.create<lv::Label>();
  side_title.add_style(heading);
  side_title.set_text(std::string_view{"Quick Actions"});
  side_title.set_pos(10, 5);

  static constexpr std::string_view actions[] = {"Restart", "Update", "Backup",
                                                  "Export"};
  for (int i = 0; i < 4; ++i) {
    auto& btn = side.create<lv::Container>();
    btn.set_size(190, 32);
    btn.set_pos(12, 35 + i * 40);
    btn.add_style(chip_styles[i % 6]);

    auto& lbl = btn.create<lv::Label>();
    lbl.add_style(tab_active);
    lbl.set_text(actions[i]);
    lbl.align(lv::Align::Center);
  }
}

using BatchFn = void (*)(lv::Object&);
constexpr BatchFn batches[] = {
    batch_buttons,  batch_cards,    batch_bars,   batch_switches,
    batch_inputs,   batch_grid,     batch_tabs,   batch_list,
    batch_arcs,     batch_mixed,
};
constexpr int NUM_BATCHES = static_cast<int>(std::size(batches));

static constexpr const char* batch_names[] = {
    "Buttons",    "Cards",     "Progress Bars", "Switches",
    "Text Inputs", "Color Grid", "Tab Layout",  "Item List",
    "Arc Gauges", "Mixed Widgets",
};

}  // namespace

int main() {
  sdl2::init(DW, DH, "Demo Stress");
  init_styles();

  std::vector<lv::Display::Pixel> buf(
      static_cast<std::size_t>(DW) * static_cast<std::size_t>(DH));
  lv::Display display{DW, DH, sdl2::flush_cb};
  display.set_draw_buffers(buf);
  display.set_render_mode(lv::RenderMode::Full);

  auto& scr = display.active_screen();
  scr.add_style(scr_bg);

  int frame = 0;
  int cycle = 0;

  lv::Ticker ticker;
  while (sdl2::ctx.running) {
    sdl2::poll_events();
    ticker.update();

    if (frame % FRAMES_PER_CYCLE == 0) {
      scr.remove_all_children();

      int idx = cycle % NUM_BATCHES;
      batches[idx](scr);

      std::printf("[stress] cycle %d — %s (%d children)\n", cycle,
                  batch_names[idx],
                  static_cast<int>(scr.child_count()));
      ++cycle;
    }

    display.refresh();
    ++frame;
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
