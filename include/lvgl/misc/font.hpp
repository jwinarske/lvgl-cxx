// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — misc/font.hpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// Font abstraction mirroring LVGL's lv_font_t.
//
// A Font provides glyph metrics and bitmap data for text rendering.
// The built-in font is an 8x16 monospace bitmap covering ASCII 32-126.
// Users can subclass Font to provide custom fonts (FreeType, etc.).

#pragma once

#include <cstdint>

namespace lv {

// ── Glyph descriptor (mirrors lv_font_glyph_dsc_t) ─────────────────────────
struct GlyphDsc {
  uint16_t advance_w = 0;           // horizontal advance in pixels
  uint16_t box_w = 0;               // bounding box width
  uint16_t box_h = 0;               // bounding box height
  int16_t ofs_x = 0;                // x offset from cursor
  int16_t ofs_y = 0;                // y offset from baseline (negative = above)
  uint8_t bpp = 1;                  // bits per pixel (1, 2, 4, or 8)
  const uint8_t* bitmap = nullptr;  // pointer to glyph bitmap data
};

// ── Font base class (mirrors lv_font_t) ─────────────────────────────────────
class Font {
 public:
  virtual ~Font() = default;

  // Look up a glyph by Unicode code point.
  // Returns true and fills `dsc` if the glyph is available.
  [[nodiscard]] virtual bool get_glyph_dsc(GlyphDsc& dsc,
                                           uint32_t unicode_letter) const = 0;

  // Font metrics
  [[nodiscard]] virtual int32_t line_height() const noexcept = 0;
  [[nodiscard]] virtual int32_t base_line() const noexcept = 0;

 protected:
  Font() = default;
};

// ── Built-in fonts ──────────────────────────────────────────────────────────

// 8x16 monospace bitmap font covering ASCII 32-126.
// 1 bpp, no antialiasing.  Suitable for basic text rendering and testing.
const Font& font_builtin_8x16() noexcept;

// Montserrat proportional fonts (4 bpp anti-aliased, ported from LVGL v9.5.0).
const Font& font_montserrat_14() noexcept;
const Font& font_montserrat_20() noexcept;
const Font& font_montserrat_24() noexcept;
const Font& font_montserrat_26() noexcept;

// Default font used when no font is specified.
inline const Font& font_default() noexcept {
  return font_montserrat_14();
}

}  // namespace lv
