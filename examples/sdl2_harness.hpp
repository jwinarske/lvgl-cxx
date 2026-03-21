// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Joel Winarske
//
// Shared SDL2 harness for lvgl-cxx demo applications.
// Provides window creation, event loop, and flush callback.
//
// Usage:
//   void create_ui(lv::Display& display);  // user implements this
//   int main() { return sdl2_run("Title", 480, 320, create_ui); }

#pragma once

#include <chrono>
#include <cstdio>
#include <functional>
#include <thread>

#include "lvgl/core/display.hpp"
#include "lvgl/core/event.hpp"
#include "lvgl/core/screen.hpp"
#include "lvgl/misc/color.hpp"
#include "lvgl/tick/tick.hpp"

#if __has_include(<SDL2/SDL.h>)
#include <SDL2/SDL.h>
#define HAS_SDL2 1
#else
#define HAS_SDL2 0
#endif

#if HAS_SDL2

namespace sdl2 {

struct Context {
  SDL_Window* window = nullptr;
  SDL_Renderer* renderer = nullptr;
  SDL_Texture* texture = nullptr;
  int32_t width = 0;
  int32_t height = 0;
  bool running = true;

  // Mouse state for input
  int32_t mouse_x = 0;
  int32_t mouse_y = 0;
  bool mouse_pressed = false;
  bool mouse_clicked = false;  // single-shot click flag
};

inline Context ctx;

inline void init(const int32_t w, const int32_t h, const char* title) {
  ctx.width = w;
  ctx.height = h;
  SDL_Init(SDL_INIT_VIDEO);
  ctx.window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED,
                                SDL_WINDOWPOS_CENTERED, w, h, 0);
  ctx.renderer = SDL_CreateRenderer(ctx.window, -1, SDL_RENDERER_ACCELERATED);
  ctx.texture = SDL_CreateTexture(ctx.renderer, SDL_PIXELFORMAT_ARGB8888,
                                  SDL_TEXTUREACCESS_STREAMING, w, h);
}

inline void deinit() {
  if (ctx.texture)
    SDL_DestroyTexture(ctx.texture);
  if (ctx.renderer)
    SDL_DestroyRenderer(ctx.renderer);
  if (ctx.window)
    SDL_DestroyWindow(ctx.window);
  SDL_Quit();
}

inline void flush_cb(lv::Display& /*disp*/, const lv::Area& area,
                     lv::BufferView2D<lv::Display::Pixel> view) {
  const SDL_Rect rect{area.x1, area.y1, area.width(), area.height()};
  SDL_UpdateTexture(ctx.texture, &rect, view.data,
                    area.width() *
                        static_cast<int>(sizeof(lv::Display::Pixel)));
  SDL_RenderCopy(ctx.renderer, ctx.texture, nullptr, nullptr);
  SDL_RenderPresent(ctx.renderer);
}

inline void poll_events() {
  ctx.mouse_clicked = false;
  SDL_Event ev;
  while (SDL_PollEvent(&ev)) {
    switch (ev.type) {
      case SDL_QUIT:
        ctx.running = false;
        break;
      case SDL_MOUSEMOTION:
        ctx.mouse_x = ev.motion.x;
        ctx.mouse_y = ev.motion.y;
        break;
      case SDL_MOUSEBUTTONDOWN:
        ctx.mouse_pressed = true;
        ctx.mouse_clicked = true;
        ctx.mouse_x = ev.button.x;
        ctx.mouse_y = ev.button.y;
        break;
      case SDL_MOUSEBUTTONUP:
        ctx.mouse_pressed = false;
        break;
      default:
        break;
    }
  }
}

// Hit-test: check if click is inside an object's screen bounds
inline bool hit_test(const lv::Object& obj, const int32_t mx,
                     const int32_t my) {
  // Walk up the parent chain to get absolute position
  int32_t ax = obj.x();
  int32_t ay = obj.y();
  const lv::Object* p = obj.parent();
  while (p) {
    ax += p->x();
    ay += p->y();
    p = p->parent();
  }
  return mx >= ax && mx < ax + obj.width() && my >= ay &&
         my < ay + obj.height();
}

using CreateFn = std::function<void(lv::Display&)>;

inline int run(const char* title,
               const int32_t w,
               const int32_t h,
               const CreateFn& create_ui) {
  init(w, h, title);

  std::vector<lv::Display::Pixel> buf(static_cast<std::size_t>(w) *
                                      static_cast<std::size_t>(h));
  lv::Display display{w, h, flush_cb};
  display.set_draw_buffers(buf);
  display.set_render_mode(lv::RenderMode::Full);

  create_ui(display);

  display.refresh();

  lv::Ticker ticker;
  while (ctx.running) {
    poll_events();
    ticker.update();
    display.refresh();
    std::this_thread::sleep_for(std::chrono::milliseconds(16));
  }

  deinit();
  return 0;
}

}  // namespace sdl2

#endif  // HAS_SDL2
