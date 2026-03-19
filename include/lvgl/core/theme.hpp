// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — core/theme.hpp
// Upstream LVGL baseline: v9.5.0  https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// Abstract base class for LVGL themes — mirrors lv_theme_t in LVGL v9.5.0
// (src/core/lv_theme.c).  Themes call obj.add_style() on freshly created
// widgets to apply a coherent visual identity.
//
// Phase 2 provides two built-in themes:
//   DefaultTheme — material-inspired blue palette (matches LVGL's default)
//   MonoTheme    — 1-bit monochrome palette for e-paper / LCDs

#pragma once

#include <cstdint>

#include "../misc/color.hpp"  // Color
#include "style.hpp"           // Style, prop::*

namespace lv {

class Object;

// ── Theme (abstract base) ─────────────────────────────────────────────────────
class Theme {
public:
    Theme()                     = default;
    Theme(const Theme&)         = default;
    Theme(Theme&&) noexcept     = default;
    Theme& operator=(const Theme&) = default;
    Theme& operator=(Theme&&) noexcept = default;
    virtual ~Theme()            = default;

    /// Called once for each newly created object.
    /// Implementations call obj.add_style(…) to apply default styles.
    virtual void apply(Object& obj) = 0;

    /// Primary colour used by the theme (buttons, focus rings, etc.)
    [[nodiscard]] virtual Color primary_color()   const noexcept;
    /// Secondary / accent colour
    [[nodiscard]] virtual Color secondary_color() const noexcept;
    /// Foreground text colour
    [[nodiscard]] virtual Color fg_color()        const noexcept;
    /// Background colour
    [[nodiscard]] virtual Color bg_color()        const noexcept;
};

// ── DefaultTheme ──────────────────────────────────────────────────────────────
// Material-inspired blue palette, matching LVGL v9.5.0's built-in default
// theme (src/themes/default/lv_theme_default.c).
//
// Base styles applied to every Object:
//   • Rounded corners (radius = 6)
//   • White background, 100% opacity
//   • Thin grey border
//
// Pressed state adds:
//   • 20% darker background
//
// Focused state adds:
//   • 3 px primary-colour outline
class DefaultTheme : public Theme {
public:
    // Construct with optional palette overrides.
    // Passing Color{} (default) keeps the built-in blue palette.
    explicit DefaultTheme(Color primary   = Color::from_hex(0x2196F3),
                          Color secondary = Color::from_hex(0xF5F5F5)) noexcept;

    void apply(Object& obj) override;

    [[nodiscard]] Color primary_color()   const noexcept override;
    [[nodiscard]] Color secondary_color() const noexcept override;
    [[nodiscard]] Color fg_color()        const noexcept override;
    [[nodiscard]] Color bg_color()        const noexcept override;

private:
    Color primary_;
    Color secondary_;

    // Pre-built styles — allocated once, shared across all objects.
    Style style_base_;
    Style style_pressed_;
    Style style_focused_;
};

// ── MonoTheme ─────────────────────────────────────────────────────────────────
// Pure black/white palette, suitable for 1-bit and greyscale displays.
// Mirrors lv_theme_mono in LVGL v9.5.0 (src/themes/mono/lv_theme_mono.c).
//
// Applied styles:
//   • Black border (1 px), transparent background
//   • Pressed state: inverted (black bg, white text)
//   • Focused state: dashed border
class MonoTheme : public Theme {
public:
    MonoTheme() noexcept;

    void apply(Object& obj) override;

    [[nodiscard]] Color primary_color()   const noexcept override;
    [[nodiscard]] Color secondary_color() const noexcept override;
    [[nodiscard]] Color fg_color()        const noexcept override;
    [[nodiscard]] Color bg_color()        const noexcept override;

private:
    Style style_base_;
    Style style_pressed_;
    Style style_focused_;
};

}  // namespace lv
