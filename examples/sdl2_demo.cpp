// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Joel Winarske
//
// SDL2 demo application for lvgl-cxx.
// Builds only when SDL2 is available.
//
// Creates a 480x320 display with a button.
// Clicking anywhere in the window sends a Clicked event to the button,
// which cycles the button color through a palette and updates a counter
// label (visible once text rendering is implemented).
//
// Build:
//   meson setup buildDir
//   meson compile -C buildDir
//   ./buildDir/sdl2_demo

#include <array>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <thread>

#include "lvgl/core/display.hpp"
#include "lvgl/core/event.hpp"
#include "lvgl/core/screen.hpp"
#include "lvgl/core/style.hpp"
#include "lvgl/tick/tick.hpp"
#include "lvgl/widgets/button.hpp"
#include "lvgl/widgets/label.hpp"

#if __has_include(<SDL2/SDL.h>)
#include <SDL2/SDL.h>
#define HAS_SDL2 1
#else
#define HAS_SDL2 0
#endif

#if HAS_SDL2

namespace {

constexpr int32_t DISP_W = 480;
constexpr int32_t DISP_H = 320;

SDL_Window* window = nullptr;
SDL_Renderer* sdl_renderer = nullptr;
SDL_Texture* texture = nullptr;

void sdl_init() {
  SDL_Init(SDL_INIT_VIDEO);
  window = SDL_CreateWindow("lvgl-cxx SDL2 Demo", SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, DISP_W, DISP_H, 0);
  sdl_renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  texture = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_ARGB8888,
                              SDL_TEXTUREACCESS_STREAMING, DISP_W, DISP_H);
}

void sdl_deinit() {
  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(sdl_renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

void flush_cb(lv::Display& /*disp*/, const lv::Area& area,
              const lv::BufferView2D<lv::Display::Pixel> view) {
  const SDL_Rect rect{area.x1, area.y1, area.width(), area.height()};
  SDL_UpdateTexture(texture, &rect, view.data,
                    area.width() *
                        static_cast<int>(sizeof(lv::Display::Pixel)));
  SDL_RenderCopy(sdl_renderer, texture, nullptr, nullptr);
  SDL_RenderPresent(sdl_renderer);
}

constexpr lv::Color palette[] = {
    lv::Color::from_hex(0x2196F3),  // blue
    lv::Color::from_hex(0x4CAF50),  // green
    lv::Color::from_hex(0xFF9800),  // orange
    lv::Color::from_hex(0xE91E63),  // pink
    lv::Color::from_hex(0x9C27B0),  // purple
    lv::Color::from_hex(0x00BCD4),  // cyan
};
constexpr int palette_size = static_cast<int>(std::size(palette));

}  // namespace

int main() {
  sdl_init();

  // Draw buffer
  std::array<lv::Display::Pixel, DISP_W * DISP_H> buf{};

  // Display
  lv::Display display{DISP_W, DISP_H, flush_cb};
  display.set_draw_buffers(buf);
  display.set_render_mode(lv::RenderMode::Full);

  // Screen background
  lv::Style scr_style;
  scr_style.set(lv::prop::BgColor{}, lv::Color::from_hex(0xF0F0F0));

  auto& scr = display.active_screen();
  scr.add_style(scr_style);

  // Button
  lv::Style btn_style;
  btn_style.set(lv::prop::BgColor{}, palette[0])
      .set(lv::prop::Radius{}, 12);

  auto& btn = scr.create<lv::Button>();
  btn.set_size(200, 80).align(lv::Align::Center);
  btn.add_style(btn_style);

  // Label style — white text on the colored button
  lv::Style lbl_style;
  lbl_style.set(lv::prop::TextColor{}, lv::Color::White());

  auto& lbl = btn.create<lv::Label>();
  lbl.add_style(lbl_style);
  lbl.set_text(std::string_view{"Clicks: 0"});
  lbl.align(lv::Align::Center);

  // Pre-build a style per palette color so they outlive the handler
  std::array<lv::Style, palette_size> btn_styles;
  for (int i = 0; i < palette_size; ++i) {
    btn_styles[static_cast<std::size_t>(i)]
        .set(lv::prop::BgColor{}, palette[i])
        .set(lv::prop::Radius{}, 12);
  }

  // Click handler — cycles button color through the palette
  int click_count = 0;
  btn.on(lv::EventCode::Clicked, [&](lv::Event&) {
       ++click_count;
       lbl.set_text("Clicks: " + std::to_string(click_count));
       lbl.align(lv::Align::Center);  // re-center after size change

       // Cycle button color so clicks are visible
       btn.remove_all_styles();
       btn.add_style(
           btn_styles[static_cast<std::size_t>(click_count % palette_size)]);

       std::printf("Click #%d\n", click_count);
     })
      .release();

  // Initial render
  display.refresh();

  // Main loop
  lv::Ticker ticker;
  bool running = true;
  while (running) {
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
      switch (ev.type) {
        case SDL_QUIT:
          running = false;
          break;
        case SDL_MOUSEBUTTONDOWN:
          btn.send_event(lv::EventCode::Clicked);
          break;
        default:
          break;
      }
    }

    ticker.update();
    display.refresh();
    std::this_thread::sleep_for(std::chrono::milliseconds(16));
  }

  sdl_deinit();
  return 0;
}

#else  // !HAS_SDL2

int main() {
  std::puts(
      "SDL2 not available. Install libsdl2-dev and rebuild.");
  return 1;
}

#endif
