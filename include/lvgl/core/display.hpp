// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — core/display.hpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0

#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <span>

#include "../misc/area.hpp"
#include "../misc/color.hpp"
#include "../misc/function.hpp"  // lv::UniqueFunction

namespace lv {

// Forward declarations
class Display;
class Object;
class Screen;
class Event;
class EventHandle;
class Layer;
enum class EventCode : uint32_t;
enum class AnimEnable : uint8_t;

// ── BufferView2D — lightweight non-owning 2D view over pixel data ───────────
// Replaces std::mdspan<Pixel, dextents<int32_t,2>> until libstdc++ ships it.
template <typename Pixel>
struct BufferView2D {
  const Pixel* data;
  int32_t rows;
  int32_t cols;

  [[nodiscard]] const Pixel& operator()(int32_t r, int32_t c) const noexcept {
    return data[r * cols + c];
  }
};

// ── Display rotation
// ──────────────────────────────────────────────────────────
enum class DisplayRotation : uint8_t { None = 0, Deg90, Deg180, Deg270 };

// ── Render mode
// ───────────────────────────────────────────────────────────────
enum class RenderMode : uint8_t { Partial, Full, Direct };

// ── Screen load animation
// ─────────────────────────────────────────────────────
enum class ScreenLoadAnim : uint8_t {
  None = 0,
  MoveLeft,
  MoveRight,
  MoveTop,
  MoveBottom,
  FadeIn,
  FadeOut,
  OverLeft,
  OverRight,
  OverTop,
  OverBottom,
};

// ── FlushCallback concept
// ───────────────────────────────────────────────────── The user-supplied
// callback that copies a rendered buffer region to hardware.
template <typename F, typename Pixel>
concept FlushCallback =
    std::invocable<F, Display&, const Area&, BufferView2D<Pixel>>;

// ── Display
// ───────────────────────────────────────────────────────────────────
class Display {
 public:
  using Pixel = color::Active::pixel_type;

  // Construct with resolution and a flush callback
  template <typename F>
    requires FlushCallback<F, Pixel>
  Display(int32_t w, int32_t h, F&& flush_cb)
      : Display(w,
                h,
                std::function<void(Display&, const Area&, BufferView2D<Pixel>)>{
                    std::forward<F>(flush_cb)}) {}

  ~Display();

  // Non-copyable, non-movable
  Display(const Display&) = delete;
  Display& operator=(const Display&) = delete;

  // ── Screens ───────────────────────────────────────────────────────────────
  [[nodiscard]] Screen& active_screen() noexcept;
  [[nodiscard]] const Screen& active_screen() const noexcept;

  Screen& load_screen(std::unique_ptr<Screen> scr,
                      ScreenLoadAnim anim = ScreenLoadAnim::None,
                      uint32_t duration = 0,
                      uint32_t delay = 0);
  Screen& create_screen();

  // ── Geometry ──────────────────────────────────────────────────────────────
  [[nodiscard]] int32_t horizontal_resolution() const noexcept;
  [[nodiscard]] int32_t vertical_resolution() const noexcept;
  void set_resolution(int32_t w, int32_t h);
  void set_rotation(DisplayRotation r);
  [[nodiscard]] DisplayRotation rotation() const noexcept;

  // ── Draw buffers ──────────────────────────────────────────────────────────
  // Single or double-buffer.  Spans must outlive the Display.
  void set_draw_buffers(std::span<Pixel> buf1, std::span<Pixel> buf2 = {});

  // ── Render ────────────────────────────────────────────────────────────────
  void refresh();
  void set_render_mode(RenderMode m);
  void set_antialiasing(bool en);

  // ── Backlight (no-op if driver doesn't support) ───────────────────────────
  void set_backlight(uint8_t level);

  // ── Events on the display itself ──────────────────────────────────────────
  [[nodiscard]] EventHandle on(EventCode code,
                               UniqueFunction<void(Event&)> handler);

 private:
  using FlushFn =
      std::function<void(Display&, const Area&, BufferView2D<Pixel>)>;

  // Private constructor used by the template constructor above
  Display(int32_t w, int32_t h, FlushFn flush_fn);

  static void collect_draw_tasks(Object& obj,
                                 Layer& layer,
                                 const Area& clip,
                                 Point origin = {0, 0});

  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace lv
