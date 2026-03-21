// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Joel Winarske
//
// Port of lv_demo_music — Music Player
// Dark-themed UI with a song list and a "Now Playing" panel at the bottom.
// Clicking the play/pause button toggles state and updates the label.

#include "sdl2_harness.hpp"

#include "lvgl/widgets/bar.hpp"
#include "lvgl/widgets/button.hpp"
#include "lvgl/widgets/container.hpp"
#include "lvgl/widgets/label.hpp"
#include "lvgl/widgets/list.hpp"

#if HAS_SDL2

namespace {

// ── Song data ───────────────────────────────────────────────────────────────

struct Song {
  const char* title;
  const char* artist;
};

constexpr Song playlist[] = {
    {"Waiting for Love", "Avicii"},
    {"Faded", "Alan Walker"},
    {"Lean On", "Major Lazer & DJ Snake"},
    {"Closer", "The Chainsmokers ft. Halsey"},
    {"Something Just Like This", "Coldplay & The Chainsmokers"},
};
constexpr int NUM_SONGS = static_cast<int>(std::size(playlist));

// ── Styles ──────────────────────────────────────────────────────────────────

lv::Style scr_style;
lv::Style dark_panel;
lv::Style song_list_style;
lv::Style title_style;
lv::Style subtitle_style;
lv::Style now_playing_style;
lv::Style btn_style;
lv::Style btn_label_style;
lv::Style progress_style;

void init_styles() {
  // Dark background
  scr_style.set(lv::prop::BgColor{}, lv::Color::from_hex(0x343247));

  // Slightly lighter panel
  dark_panel.set(lv::prop::BgColor{}, lv::Color::from_hex(0x3D3B54))
      .set(lv::prop::Radius{}, 12)
      .set(lv::prop::PadTop{}, 10)
      .set(lv::prop::PadBottom{}, 10)
      .set(lv::prop::PadLeft{}, 12)
      .set(lv::prop::PadRight{}, 12);

  song_list_style.set(lv::prop::BgColor{}, lv::Color::from_hex(0x3D3B54))
      .set(lv::prop::Radius{}, 8);

  title_style.set(lv::prop::TextColor{}, lv::Color::White());

  subtitle_style.set(lv::prop::TextColor{}, lv::Color::from_hex(0xAAAAAA));

  now_playing_style.set(lv::prop::TextColor{}, lv::Color::from_hex(0xE0E0E0));

  btn_style.set(lv::prop::BgColor{}, lv::Color::from_hex(0x6C63FF))
      .set(lv::prop::Radius{}, 20);

  btn_label_style.set(lv::prop::TextColor{}, lv::Color::White());

  progress_style.set(lv::prop::BgColor{}, lv::Color::from_hex(0x6C63FF));
}

}  // namespace

void create_ui(lv::Display& display) {
  init_styles();

  auto& scr = display.active_screen();
  scr.add_style(scr_style);

  // ── Title ─────────────────────────────────────────────────────────────────
  auto& header = scr.create<lv::Label>();
  header.add_style(title_style);
  header.set_text(std::string_view{"Music Player"});
  header.set_pos(15, 8);

  // ── Song list (top section) ───────────────────────────────────────────────
  auto& list = scr.create<lv::List>();
  list.set_size(450, 180);
  list.set_pos(15, 35);
  list.add_style(song_list_style);

  list.add_text("Playlist");
  for (int i = 0; i < NUM_SONGS; ++i) {
    std::string entry =
        std::string(playlist[i].title) + " - " + playlist[i].artist;
    list.add_btn("", entry);
  }

  // ── Now Playing panel (bottom section) ────────────────────────────────────
  auto& panel = scr.create<lv::Container>();
  panel.set_size(450, 90);
  panel.set_pos(15, 222);
  panel.add_style(dark_panel);

  auto& np_label = panel.create<lv::Label>();
  np_label.add_style(now_playing_style);
  np_label.set_text(std::string_view{"Now Playing"});
  np_label.set_pos(0, 0);

  // Current song title
  auto& song_title = panel.create<lv::Label>();
  song_title.add_style(title_style);
  song_title.set_text(std::string_view{playlist[0].title});
  song_title.set_pos(0, 22);

  // Artist
  auto& artist_label = panel.create<lv::Label>();
  artist_label.add_style(subtitle_style);
  artist_label.set_text(std::string_view{playlist[0].artist});
  artist_label.set_pos(0, 42);

  // Progress bar
  auto& progress = panel.create<lv::Bar>();
  progress.set_size(260, 6);
  progress.set_pos(0, 65);
  progress.set_range(0, 100);
  progress.set_value(35);

  // Play/Pause button
  auto& play_btn = panel.create<lv::Button>();
  play_btn.set_size(70, 40);
  play_btn.set_pos(340, 20);
  play_btn.add_style(btn_style);

  auto& play_lbl = play_btn.create<lv::Label>();
  play_lbl.add_style(btn_label_style);
  play_lbl.set_text(std::string_view{"Pause"});
  play_lbl.align(lv::Align::Center);

  // Toggle play/pause on click
  static bool playing = true;
  play_btn
      .on(lv::EventCode::Clicked,
          [&play_lbl](lv::Event&) {
            playing = !playing;
            play_lbl.set_text(
                std::string_view{playing ? "Pause" : "Play "});
            play_lbl.align(lv::Align::Center);
            std::printf("[music] %s\n", playing ? "Playing" : "Paused");
          })
      .release();
}

int main() {
  return sdl2::run("Demo Music", 480, 320, create_ui);
}

#else

#include <cstdio>

int main() {
  std::puts("SDL2 not available. Install libsdl2-dev and rebuild.");
  return 1;
}

#endif
