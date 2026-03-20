// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Joel Winarske
//
// Port of lv_demo_benchmark — Benchmark Scenes
// Cycles through 6 rendering scenes, each running for 3 seconds.
// Measures actual render time (no frame limiter).
// Prints FPS and per-frame render time to stdout.

#include "sdl2_harness.hpp"

#include "lvgl/widgets/arc.hpp"
#include "lvgl/widgets/container.hpp"
#include "lvgl/widgets/label.hpp"

#if HAS_SDL2

namespace {

constexpr int32_t DISP_W = 480;
constexpr int32_t DISP_H = 320;
constexpr int SCENE_COUNT = 6;
constexpr auto SCENE_DURATION = std::chrono::seconds(3);

// Persistent styles
lv::Style bg_styles[4];
lv::Style rect_style;
lv::Style label_style;
lv::Style card_style;
lv::Style card_text_style;

void init_styles() {
  bg_styles[0].set(lv::prop::BgColor{}, lv::Color::from_hex(0xE91E63));
  bg_styles[1].set(lv::prop::BgColor{}, lv::Color::from_hex(0x2196F3));
  bg_styles[2].set(lv::prop::BgColor{}, lv::Color::from_hex(0x4CAF50));
  bg_styles[3].set(lv::prop::BgColor{}, lv::Color::from_hex(0xFF9800));

  rect_style.set(lv::prop::BgColor{}, lv::Color::from_hex(0x3F51B5))
      .set(lv::prop::Radius{}, 4);

  label_style.set(lv::prop::TextColor{}, lv::Color::from_hex(0x212121));

  card_style.set(lv::prop::BgColor{}, lv::Color::White())
      .set(lv::prop::Radius{}, 8)
      .set(lv::prop::PadTop{}, 8)
      .set(lv::prop::PadBottom{}, 8)
      .set(lv::prop::PadLeft{}, 8)
      .set(lv::prop::PadRight{}, 8);

  card_text_style.set(lv::prop::TextColor{}, lv::Color::from_hex(0x333333));
}

// ── Scene setup (called ONCE per scene) ─────────────────────────────────────

void setup_solid_bg(lv::Object& scr) {
  scr.remove_all_styles();
  scr.add_style(bg_styles[0]);
}

void setup_large_rect(lv::Object& scr) {
  auto& rect = scr.create<lv::Container>();
  rect.set_size(DISP_W / 2, DISP_H / 2);
  rect.align(lv::Align::Center);
  rect.add_style(rect_style);
}

void setup_grid_rects(lv::Object& scr) {
  constexpr int32_t margin = 10;
  constexpr int32_t cell_w = (DISP_W - 4 * margin) / 3;
  constexpr int32_t cell_h = (DISP_H - 4 * margin) / 3;
  for (int r = 0; r < 3; ++r) {
    for (int c = 0; c < 3; ++c) {
      auto& rect = scr.create<lv::Container>();
      rect.set_size(cell_w, cell_h);
      rect.set_pos(margin + c * (cell_w + margin),
                    margin + r * (cell_h + margin));
      rect.add_style(rect_style);
    }
  }
}

void setup_labels(lv::Object& scr) {
  for (int i = 0; i < 20; ++i) {
    auto& lbl = scr.create<lv::Label>();
    lbl.add_style(label_style);
    lbl.set_text(std::string_view{"Hello LVGL!"});
    lbl.set_pos(10 + (i % 5) * 92, 10 + (i / 5) * 22);
  }
}

void setup_arcs(lv::Object& scr) {
  for (int i = 0; i < 9; ++i) {
    auto& arc = scr.create<lv::Arc>();
    arc.set_size(80, 80);
    arc.set_pos(20 + (i % 3) * 150, 20 + (i / 3) * 100);
    arc.set_range(0, 100);
    arc.set_value(static_cast<int32_t>((i + 1) * 10));
    arc.set_bg_start_angle(135);
    arc.set_bg_end_angle(45);
  }
}

void setup_cards(lv::Object& scr) {
  static constexpr std::string_view titles[] = {
      "Dashboard", "Settings", "Profile",
      "Messages",  "Analytics", "Help",
      "Logs",      "Storage",   "Network"};
  for (int i = 0; i < 9; ++i) {
    auto& card = scr.create<lv::Container>();
    card.set_size(130, 80);
    card.set_pos(15 + (i % 3) * 150, 15 + (i / 3) * 95);
    card.add_style(card_style);

    auto& lbl = card.create<lv::Label>();
    lbl.add_style(card_text_style);
    lbl.set_text(titles[i]);
    lbl.align(lv::Align::Center);
  }
}

// ── Per-frame update (called every frame, only for animated scenes) ─────────

void update_solid_bg(lv::Object& scr, const int frame) {
  scr.remove_all_styles();
  scr.add_style(bg_styles[(frame / 8) % 4]);
}

using SetupFn = void (*)(lv::Object&);
using UpdateFn = void (*)(lv::Object&, int);

struct SceneDesc {
  const char* name;
  SetupFn setup;
  UpdateFn update;  // nullptr = static scene
};

constexpr SceneDesc scenes[SCENE_COUNT] = {
    {"Solid background", setup_solid_bg, update_solid_bg},
    {"Large rectangle", setup_large_rect, nullptr},
    {"3x3 grid", setup_grid_rects, nullptr},
    {"Labels (20)", setup_labels, nullptr},
    {"Arcs (9)", setup_arcs, nullptr},
    {"Cards (9)", setup_cards, nullptr},
};

}  // namespace

int main() {
  sdl2::init(DISP_W, DISP_H, "Demo Benchmark");
  init_styles();

  std::vector<lv::Display::Pixel> buf(
      static_cast<std::size_t>(DISP_W) * static_cast<std::size_t>(DISP_H));
  lv::Display display{DISP_W, DISP_H, sdl2::flush_cb};
  display.set_draw_buffers(buf);
  display.set_render_mode(lv::RenderMode::Full);

  static lv::Style base_bg;
  base_bg.set(lv::prop::BgColor{}, lv::Color::from_hex(0xF0F0F0));
  auto& scr = display.active_screen();
  scr.add_style(base_bg);

  std::printf("[benchmark] Display: %dx%d, Full render mode, no frame limiter\n",
              DISP_W, DISP_H);
  std::printf("[benchmark] Each scene runs for 3 seconds\n\n");

  for (int scene_idx = 0; scene_idx < SCENE_COUNT && sdl2::ctx.running;
       ++scene_idx) {
    // Reset screen
    scr.remove_all_children();
    scr.remove_all_styles();
    scr.add_style(base_bg);

    // Setup scene (one-time)
    scenes[scene_idx].setup(scr);
    display.refresh();  // warm-up frame

    // Benchmark loop — NO sleep, measure raw render throughput
    int frames = 0;
    auto start = std::chrono::steady_clock::now();

    while (sdl2::ctx.running) {
      sdl2::poll_events();

      // Per-frame update for animated scenes
      if (scenes[scene_idx].update)
        scenes[scene_idx].update(scr, frames);

      display.refresh();
      auto t1 = std::chrono::steady_clock::now();

      ++frames;

      if (auto elapsed = t1 - start; elapsed >= SCENE_DURATION)
        break;

      // Yield to prevent OS starvation, but don't throttle
      std::this_thread::yield();
    }

    auto end = std::chrono::steady_clock::now();
    const double secs = std::chrono::duration<double>(end - start).count();
    const double fps = static_cast<double>(frames) / secs;
    const double ms_per_frame = secs * 1000.0 / static_cast<double>(frames);

    std::printf("[benchmark] Scene %d (%s): %.0f FPS (%.2f ms/frame, %d frames)\n",
                scene_idx + 1, scenes[scene_idx].name, fps, ms_per_frame,
                frames);
  }

  std::printf("\n[benchmark] All scenes complete.\n");
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
