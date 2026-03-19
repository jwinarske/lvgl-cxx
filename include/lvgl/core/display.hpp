// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — core/display.hpp
// Upstream LVGL baseline: v9.5.0  https://github.com/lvgl/lvgl/releases/tag/v9.5.0

#pragma once

#include <cstdint>
#include <functional>
#include <mdspan>
#include <memory>
#include <span>

#include "../misc/area.hpp"
#include "../misc/color.hpp"

namespace lv {

// Forward declarations
class Screen;
class Event;
class EventHandle;
enum class EventCode : uint32_t;
enum class AnimEnable : uint8_t;

// ── Display rotation ──────────────────────────────────────────────────────────
enum class DisplayRotation : uint8_t { None = 0, Deg90, Deg180, Deg270 };

// ── Render mode ───────────────────────────────────────────────────────────────
enum class RenderMode : uint8_t { Partial, Full, Direct };

// ── Screen load animation ─────────────────────────────────────────────────────
enum class ScreenLoadAnim : uint8_t {
    None = 0,
    MoveLeft, MoveRight, MoveTop, MoveBottom,
    FadeIn, FadeOut, OverLeft, OverRight, OverTop, OverBottom,
};

// ── FlushCallback concept ─────────────────────────────────────────────────────
// The user-supplied callback that copies a rendered buffer region to hardware.
template<typename F, typename Pixel>
concept FlushCallback =
    std::invocable<F,
        Display&,
        const Area&,
        std::mdspan<const Pixel, std::dextents<int32_t, 2>>>;

// ── Display ───────────────────────────────────────────────────────────────────
class Display {
public:
    // Construct with resolution and a flush callback (any callable satisfying
    // FlushCallback<F, color::Active::pixel_type>).
    template<typename F>
        requires FlushCallback<F, color::Active::pixel_type>
    Display(int32_t w, int32_t h, F&& flush_cb)
        : Display(w, h,
                  std::function<void(Display&,
                                     const Area&,
                                     std::mdspan<const color::Active::pixel_type,
                                                 std::dextents<int32_t,2>>)>
                      {std::forward<F>(flush_cb)}) {}

    ~Display();

    // Non-copyable, non-movable
    Display(const Display&) = delete;
    Display& operator=(const Display&) = delete;

    // ── Screens ───────────────────────────────────────────────────────────────
    [[nodiscard]] Screen&       active_screen()       noexcept;
    [[nodiscard]] const Screen& active_screen() const noexcept;

    Screen& load_screen(std::unique_ptr<Screen> scr,
                        ScreenLoadAnim anim     = ScreenLoadAnim::None,
                        uint32_t       duration = 0,
                        uint32_t       delay    = 0);
    Screen& create_screen();

    // ── Geometry ──────────────────────────────────────────────────────────────
    [[nodiscard]] int32_t horizontal_resolution() const noexcept;
    [[nodiscard]] int32_t vertical_resolution()   const noexcept;
    void set_resolution(int32_t w, int32_t h);
    void set_rotation(DisplayRotation r);
    [[nodiscard]] DisplayRotation rotation() const noexcept;

    // ── Draw buffers ──────────────────────────────────────────────────────────
    using Pixel = color::Active::pixel_type;

    // Single or double-buffer.  Spans must outlive the Display.
    void set_draw_buffers(std::span<Pixel> buf1,
                          std::span<Pixel> buf2 = {});

    // ── Render ────────────────────────────────────────────────────────────────
    void refresh();
    void set_render_mode(RenderMode m);
    void set_antialiasing(bool en);

    // ── Backlight (no-op if driver doesn't support) ───────────────────────────
    void set_backlight(uint8_t level);

    // ── Events on the display itself ──────────────────────────────────────────
    [[nodiscard]] EventHandle on(EventCode code,
                                 std::move_only_function<void(Event&)> handler);

private:
    using FlushFn = std::function<void(
        Display&,
        const Area&,
        std::mdspan<const Pixel, std::dextents<int32_t,2>>)>;

    // Private constructor used by the template constructor above
    Display(int32_t w, int32_t h, FlushFn flush_fn);

    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace lv
