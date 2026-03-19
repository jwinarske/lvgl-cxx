// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — misc/color.hpp
// Upstream LVGL baseline: v9.5.0  https://github.com/lvgl/lvgl/releases/tag/v9.5.0

#pragma once

#include <cstdint>
#include <compare>
#include "../../lvgl_config.hpp"

namespace lv {

// ── Color format tags ─────────────────────────────────────────────────────────
namespace color {

struct ARGB8888 {
    struct pixel_type { uint8_t b, g, r, a; };
    static constexpr int  bits_per_pixel = 32;
    static constexpr bool has_alpha      = true;
};

struct RGB888 {
    struct pixel_type { uint8_t b, g, r; };
    static constexpr int  bits_per_pixel = 24;
    static constexpr bool has_alpha      = false;
};

struct RGB565 {
    struct pixel_type {
        uint16_t r : 5;
        uint16_t g : 6;
        uint16_t b : 5;
    };
    static constexpr int  bits_per_pixel = 16;
    static constexpr bool has_alpha      = false;
};

struct L8 {
    using pixel_type = uint8_t;
    static constexpr int  bits_per_pixel = 8;
    static constexpr bool has_alpha      = false;
};

template<typename T>
concept ColorFormatTag = requires {
    typename T::pixel_type;
    { T::bits_per_pixel } -> std::convertible_to<int>;
    { T::has_alpha      } -> std::convertible_to<bool>;
};

static_assert(ColorFormatTag<ARGB8888>);
static_assert(ColorFormatTag<RGB888>);
static_assert(ColorFormatTag<RGB565>);
static_assert(ColorFormatTag<L8>);

// Select the active pixel format based on meson.options / lvgl_config.hpp
#if LVGLCXX_COLOR_ARGB8888
using Active = ARGB8888;
#elif LVGLCXX_COLOR_RGB888
using Active = RGB888;
#elif LVGLCXX_COLOR_RGB565
using Active = RGB565;
#elif LVGLCXX_COLOR_L8
using Active = L8;
#else
#  error "No color format selected — check meson.options"
#endif

} // namespace color

// ── Opacity constants ─────────────────────────────────────────────────────────
inline constexpr uint8_t OpaTransp = 0;
inline constexpr uint8_t OpaFull   = 255;

// ── Color (32-bit ARGB8888 logical color) ─────────────────────────────────────
// Internal storage is always 32-bit ARGB; the renderer converts to the active
// pixel format at draw time.
struct Color {
    uint8_t r = 0, g = 0, b = 0, a = 255;

    constexpr Color() noexcept = default;
    constexpr Color(uint8_t r, uint8_t g, uint8_t b,
                    uint8_t a = 255) noexcept
        : r(r), g(g), b(b), a(a) {}

    // Construct from a 0xRRGGBB hex literal (alpha = 255)
    [[nodiscard]] static constexpr Color from_hex(uint32_t hex) noexcept {
        return Color{
            static_cast<uint8_t>((hex >> 16) & 0xFF),
            static_cast<uint8_t>((hex >>  8) & 0xFF),
            static_cast<uint8_t>( hex        & 0xFF),
        };
    }
    // Construct from 0xAARRGGBB
    [[nodiscard]] static constexpr Color from_hex32(uint32_t hex) noexcept {
        return Color{
            static_cast<uint8_t>((hex >> 16) & 0xFF),
            static_cast<uint8_t>((hex >>  8) & 0xFF),
            static_cast<uint8_t>( hex        & 0xFF),
            static_cast<uint8_t>((hex >> 24) & 0xFF),
        };
    }

    // Named colours
    static constexpr Color Black()   noexcept { return {  0,   0,   0}; }
    static constexpr Color White()   noexcept { return {255, 255, 255}; }
    static constexpr Color Red()     noexcept { return {255,   0,   0}; }
    static constexpr Color Green()   noexcept { return {  0, 255,   0}; }
    static constexpr Color Blue()    noexcept { return {  0,   0, 255}; }
    static constexpr Color Cyan()    noexcept { return {  0, 255, 255}; }
    static constexpr Color Magenta() noexcept { return {255,   0, 255}; }
    static constexpr Color Yellow()  noexcept { return {255, 255,   0}; }
    static constexpr Color Transp()  noexcept { return {  0,   0,   0, 0}; }

    [[nodiscard]] constexpr Color with_alpha(uint8_t alpha) const noexcept {
        return {r, g, b, alpha};
    }
    [[nodiscard]] constexpr Color mix(Color other, uint8_t ratio) const noexcept;

    auto operator<=>(const Color&) const = default;
};

// ── ColorFilter (gradient descriptor) ────────────────────────────────────────
enum class GradDir : uint8_t { None, Hor, Ver, Linear, Radial, Conical };

struct GradStop {
    Color    color;
    uint8_t  frac = 0;   // 0–255 position along the gradient
};

struct ColorFilter {
    GradDir  dir   = GradDir::None;
    GradStop stops[4];
    uint8_t  stop_count = 0;
};

} // namespace lv
