// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — draw/software_renderer.hpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// SoftwareRenderer — built-in CPU-based renderer that satisfies the
// Renderer concept.  Processes draw tasks into a DrawBuffer.

#pragma once

#include <cmath>
#include <concepts>
#include <span>

#include "../core/draw_buffer.hpp"
#include "../misc/color.hpp"
#include "../misc/font.hpp"
#include "descriptors.hpp"

namespace lv {

// ── Renderer concept ────────────────────────────────────────────────────────
template <typename R, typename Pixel>
concept Renderer = requires(R renderer,
                            DrawBuffer<Pixel>& buf,
                            std::span<const DrawTask> tasks) {
  { renderer.execute(buf, tasks) } -> std::same_as<void>;
};

// ── SoftwareRenderer ────────────────────────────────────────────────────────
class SoftwareRenderer {
 public:
  template <typename Pixel>
  void execute(DrawBuffer<Pixel>& buf, std::span<const DrawTask> tasks);

 private:
  template <typename Pixel>
  void render_rect(DrawBuffer<Pixel>& buf, const RectTask& task);

  template <typename Pixel>
  void render_line(DrawBuffer<Pixel>& buf, const LineTask& task);

  template <typename Pixel>
  void render_label(DrawBuffer<Pixel>& buf, const LabelTask& task);

  template <typename Pixel>
  void render_arc(DrawBuffer<Pixel>& buf, const ArcTask& task);
};

// ── Pixel conversion helper ─────────────────────────────────────────────────
namespace detail {

// Convert internal Color (ARGB8888) to target pixel format
template <typename Pixel>
constexpr Pixel color_to_pixel(Color c, uint8_t opa = 255) noexcept;

template <>
constexpr color::ARGB8888::pixel_type color_to_pixel(Color c,
                                                     uint8_t opa) noexcept {
  const auto a = static_cast<uint8_t>(static_cast<uint16_t>(c.a) *
                                      static_cast<uint16_t>(opa) / 255);
  return {c.b, c.g, c.r, a};
}

template <>
constexpr color::RGB888::pixel_type color_to_pixel(Color c,
                                                   uint8_t /*opa*/) noexcept {
  return {c.b, c.g, c.r};
}

template <>
constexpr color::RGB565::pixel_type color_to_pixel(Color c,
                                                   uint8_t /*opa*/) noexcept {
  color::RGB565::pixel_type px{};
  px.r = static_cast<uint16_t>(c.r >> 3) & 0x1Fu;
  px.g = static_cast<uint16_t>(c.g >> 2) & 0x3Fu;
  px.b = static_cast<uint16_t>(c.b >> 3) & 0x1Fu;
  return px;
}

template <>
constexpr uint8_t color_to_pixel<uint8_t>(Color c, uint8_t /*opa*/) noexcept {
  // Luminance: approximate (r*77 + g*150 + b*29) >> 8
  return static_cast<uint8_t>((static_cast<uint16_t>(c.r) * 77 +
                               static_cast<uint16_t>(c.g) * 150 +
                               static_cast<uint16_t>(c.b) * 29) >>
                              8);
}

// Alpha-blend src over dst
template <typename Pixel>
constexpr Pixel blend_pixel(Pixel dst, Pixel src, uint8_t opa) noexcept;

template <>
constexpr color::ARGB8888::pixel_type blend_pixel(
    color::ARGB8888::pixel_type dst,
    color::ARGB8888::pixel_type src,
    uint8_t opa) noexcept {
  const auto a = static_cast<uint16_t>(static_cast<uint16_t>(src.a) *
                                       static_cast<uint16_t>(opa) / 255);
  if (a == 255)
    return src;
  if (a == 0)
    return dst;
  const auto inv = static_cast<uint16_t>(255 - a);
  return {
      static_cast<uint8_t>((static_cast<uint16_t>(src.b) * a +
                            static_cast<uint16_t>(dst.b) * inv) /
                           255),
      static_cast<uint8_t>((static_cast<uint16_t>(src.g) * a +
                            static_cast<uint16_t>(dst.g) * inv) /
                           255),
      static_cast<uint8_t>((static_cast<uint16_t>(src.r) * a +
                            static_cast<uint16_t>(dst.r) * inv) /
                           255),
      static_cast<uint8_t>(a + static_cast<uint16_t>(dst.a) * inv / 255),
  };
}

}  // namespace detail

// ── Template implementation ─────────────────────────────────────────────────

template <typename Pixel>
void SoftwareRenderer::execute(DrawBuffer<Pixel>& buf,
                               std::span<const DrawTask> tasks) {
  for (const auto& task : tasks) {
    std::visit(
        [&](const auto& t) {
          using T = std::decay_t<decltype(t)>;
          if constexpr (std::same_as<T, RectTask>) {
            render_rect(buf, t);
          } else if constexpr (std::same_as<T, LineTask>) {
            render_line(buf, t);
          } else if constexpr (std::same_as<T, LabelTask>) {
            render_label(buf, t);
          } else if constexpr (std::same_as<T, ArcTask>) {
            render_arc(buf, t);
          }
        },
        task);
  }
}

template <typename Pixel>
void SoftwareRenderer::render_rect(DrawBuffer<Pixel>& buf,
                                   const RectTask& task) {
  const auto& dsc = task.dsc;
  const Area& bounds = task.bounds;
  const Area& clip = task.clip_area;

  // Compute the actual drawing area (intersection of bounds and clip)
  Area draw_area = bounds.intersect(clip);
  if (draw_area.empty())
    return;

  // Clamp to buffer dimensions
  const int32_t bw = buf.width();
  const int32_t bh = buf.height();
  draw_area.x1 = std::max(draw_area.x1, int32_t{0});
  draw_area.y1 = std::max(draw_area.y1, int32_t{0});
  draw_area.x2 = std::min(draw_area.x2, bw - 1);
  draw_area.y2 = std::min(draw_area.y2, bh - 1);
  if (draw_area.empty())
    return;

  const Pixel fill = detail::color_to_pixel<Pixel>(dsc.bg_color, dsc.bg_opa);

  if (dsc.radius <= 0) {
    // Simple filled rectangle
    for (int32_t row = draw_area.y1; row <= draw_area.y2; ++row) {
      for (int32_t col = draw_area.x1; col <= draw_area.x2; ++col) {
        if (dsc.bg_opa == 255) {
          buf[row, col] = fill;
        } else {
          buf[row, col] = detail::blend_pixel(buf[row, col], fill, dsc.bg_opa);
        }
      }
    }
  } else {
    // Rounded rectangle — simple corner-radius implementation
    const int32_t r =
        std::min(dsc.radius, std::min(bounds.width() / 2, bounds.height() / 2));

    for (int32_t row = draw_area.y1; row <= draw_area.y2; ++row) {
      for (int32_t col = draw_area.x1; col <= draw_area.x2; ++col) {
        // Check if the pixel is inside rounded rect
        const int32_t px = col - bounds.x1;
        const int32_t py = row - bounds.y1;
        const int32_t rw = bounds.width();
        const int32_t rh = bounds.height();

        bool inside = true;
        // Check corners
        int32_t cx = 0;
        int32_t cy = 0;
        if (px < r && py < r) {
          cx = r - px - 1;
          cy = r - py - 1;
        } else if (px >= rw - r && py < r) {
          cx = px - (rw - r);
          cy = r - py - 1;
        } else if (px < r && py >= rh - r) {
          cx = r - px - 1;
          cy = py - (rh - r);
        } else if (px >= rw - r && py >= rh - r) {
          cx = px - (rw - r);
          cy = py - (rh - r);
        } else {
          cx = 0;
          cy = 0;
        }

        if (cx > 0 || cy > 0) {
          // Distance from a corner center (integer approximation)
          const int64_t dist_sq =
              static_cast<int64_t>(cx) * cx + static_cast<int64_t>(cy) * cy;
          const int64_t r_sq = static_cast<int64_t>(r) * r;
          if (dist_sq > r_sq)
            inside = false;
        }

        if (inside) {
          if (dsc.bg_opa == 255) {
            buf[row, col] = fill;
          } else {
            buf[row, col] =
                detail::blend_pixel(buf[row, col], fill, dsc.bg_opa);
          }
        }
      }
    }
  }

  // Border rendering
  if (dsc.border_width > 0 && dsc.border_opa > 0) {
    const Pixel border_px =
        detail::color_to_pixel<Pixel>(dsc.border_color, dsc.border_opa);

    for (int32_t row = draw_area.y1; row <= draw_area.y2; ++row) {
      for (int32_t col = draw_area.x1; col <= draw_area.x2; ++col) {
        const int32_t px = col - bounds.x1;
        const int32_t py = row - bounds.y1;
        const int32_t rw = bounds.width();
        const int32_t rh = bounds.height();
        const int32_t bw_val = dsc.border_width;

        const bool on_border = px < bw_val || px >= rw - bw_val ||
                               py < bw_val || py >= rh - bw_val;

        if (on_border) {
          if (dsc.border_opa == 255) {
            buf[row, col] = border_px;
          } else {
            buf[row, col] =
                detail::blend_pixel(buf[row, col], border_px, dsc.border_opa);
          }
        }
      }
    }
  }
}

template <typename Pixel>
void SoftwareRenderer::render_line(DrawBuffer<Pixel>& buf,
                                   const LineTask& task) {
  const auto& dsc = task.dsc;
  const Pixel px = detail::color_to_pixel<Pixel>(dsc.color, dsc.opa);

  // Bresenham's line algorithm
  int32_t x0 = task.p1.x;
  int32_t y0 = task.p1.y;
  const int32_t x1 = task.p2.x;
  const int32_t y1 = task.p2.y;

  const int32_t dx = std::abs(x1 - x0);
  const int32_t dy = -std::abs(y1 - y0);
  const int32_t sx = x0 < x1 ? 1 : -1;
  const int32_t sy = y0 < y1 ? 1 : -1;
  int32_t err = dx + dy;

  const int32_t bw = buf.width();
  const int32_t bh = buf.height();
  const Area& clip = task.clip_area;

  for (;;) {
    if (x0 >= 0 && x0 < bw && y0 >= 0 && y0 < bh && clip.contains({x0, y0})) {
      buf[y0, x0] = px;
    }
    if (x0 == x1 && y0 == y1)
      break;
    const int32_t e2 = 2 * err;
    if (e2 >= dy) {
      err += dy;
      x0 += sx;
    }
    if (e2 <= dx) {
      err += dx;
      y0 += sy;
    }
  }
}

namespace detail {

// Extract alpha (0-255) from a packed glyph bitmap at a given pixel position.
// Handles 1, 2, 4, and 8 bpp formats as used by LVGL's lv_font_fmt_txt.
//
// LVGL stores glyph bitmaps as a flat bit stream with NO per-row padding.
// Pixel (row, col) is at bit offset (row * box_w + col) * bpp from the start.
inline uint8_t glyph_alpha(const uint8_t* bitmap,
                           uint8_t bpp,
                           int32_t box_w,
                           int32_t row,
                           int32_t col) noexcept {
  const int32_t pixel_idx = row * box_w + col;
  const int32_t bit_offset = pixel_idx * bpp;
  const int32_t byte_idx = bit_offset / 8;
  // MSB first: high bits of each byte are leftmost pixels
  const int32_t bit_ofs = (8 - bpp) - (bit_offset % 8);
  const uint8_t raw =
      static_cast<uint8_t>((bitmap[byte_idx] >> bit_ofs) & ((1u << bpp) - 1u));

  // Scale to 0-255
  switch (bpp) {
    case 1:
      return raw ? uint8_t{255} : uint8_t{0};
    case 2: {
      constexpr uint8_t tbl[] = {0, 85, 170, 255};
      return tbl[raw];
    }
    case 4:
      return static_cast<uint8_t>(raw * 17);  // 0→0, 15→255
    case 8:
      return raw;
    default:
      return 0;
  }
}

}  // namespace detail

template <typename Pixel>
void SoftwareRenderer::render_label(DrawBuffer<Pixel>& buf,
                                    const LabelTask& task) {
  const auto& dsc = task.dsc;
  const Font* font = dsc.font;
  if (!font)
    font = &font_default();

  const Pixel fg = detail::color_to_pixel<Pixel>(dsc.color, dsc.opa);
  const Area& clip = task.clip_area;
  const int32_t bw = buf.width();
  const int32_t bh = buf.height();

  int32_t cursor_x = task.pos.x;
  int32_t cursor_y = task.pos.y;
  const int32_t line_h = font->line_height();

  for (const char ch : dsc.text) {
    if (ch == '\n') {
      cursor_x = task.pos.x;
      cursor_y += line_h + dsc.line_spacing;
      continue;
    }

    GlyphDsc g{};
    if (!font->get_glyph_dsc(g, static_cast<uint32_t>(ch))) {
      cursor_x += static_cast<int32_t>(g.advance_w);
      continue;
    }

    if (!g.bitmap) {
      cursor_x += static_cast<int32_t>(g.advance_w) + dsc.letter_spacing;
      continue;
    }

    // Glyph top-left in screen coords.
    // For LVGL-style fonts: ofs_y is measured from the baseline upward,
    // so screen_y = cursor_y + (line_height - base_line - box_h - ofs_y)
    // For the built-in 8x16 font ofs_y==0 means top-aligned.
    const int32_t gx = cursor_x + g.ofs_x;
    const int32_t gy =
        (g.bpp > 1) ? cursor_y + (line_h - font->base_line() -
                                  static_cast<int32_t>(g.box_h) - g.ofs_y)
                    : cursor_y + g.ofs_y;

    for (int32_t row = 0; row < static_cast<int32_t>(g.box_h); ++row) {
      const int32_t sy = gy + row;
      if (sy < 0 || sy >= bh)
        continue;

      for (int32_t col = 0; col < static_cast<int32_t>(g.box_w); ++col) {
        const int32_t sx = gx + col;
        if (sx < 0 || sx >= bw)
          continue;
        if (!clip.contains({sx, sy}))
          continue;

        const uint8_t alpha = detail::glyph_alpha(
            g.bitmap, g.bpp, static_cast<int32_t>(g.box_w), row, col);
        if (alpha == 0)
          continue;

        const auto eff_opa =
            static_cast<uint8_t>(static_cast<uint16_t>(alpha) *
                                 static_cast<uint16_t>(dsc.opa) / 255);

        if (eff_opa == 255) {
          buf[sy, sx] = fg;
        } else {
          buf[sy, sx] = detail::blend_pixel(buf[sy, sx], fg, eff_opa);
        }
      }
    }

    cursor_x += static_cast<int32_t>(g.advance_w) + dsc.letter_spacing;
  }
}

template <typename Pixel>
void SoftwareRenderer::render_arc(DrawBuffer<Pixel>& buf, const ArcTask& task) {
  const auto& dsc = task.dsc;
  const Pixel fg = detail::color_to_pixel<Pixel>(dsc.color, dsc.opa);
  const Area& clip = task.clip_area;
  const int32_t bw = buf.width();
  const int32_t bh = buf.height();

  const int32_t cx = task.center.x;
  const int32_t cy = task.center.y;
  const int32_t r_outer = dsc.radius;
  const int32_t r_inner = std::max(0, r_outer - dsc.width);

  // Convert angles to a comparable range.
  // LVGL uses 0° = 12 o'clock, clockwise. We convert to standard math
  // convention for the atan2-based check.
  auto normalize_angle = [](int32_t a) -> int32_t {
    a = a % 360;
    if (a < 0)
      a += 360;
    return a;
  };

  const int32_t a_start = normalize_angle(dsc.start_angle);
  const int32_t a_end = normalize_angle(dsc.end_angle);

  // Check if a screen-space angle (in degrees, 0=right, CCW) falls
  // within the arc's angular span. The arc is defined clockwise from
  // start_angle with 0° at top (12 o'clock). We convert pixel angles
  // to the same coordinate system.
  auto angle_in_range = [&](int32_t px, int32_t py) -> bool {
    // atan2 gives angle from center; convert to 0°=top, clockwise
    const double dx = static_cast<double>(px - cx);
    const double dy = static_cast<double>(py - cy);
    double angle_rad = std::atan2(dx, -dy);  // 0=top, CW positive
    int32_t angle_deg =
        static_cast<int32_t>(angle_rad * 180.0 / 3.14159265358979323846);
    if (angle_deg < 0)
      angle_deg += 360;

    if (a_start <= a_end) {
      return angle_deg >= a_start && angle_deg <= a_end;
    }
    // Wraps around 360
    return angle_deg >= a_start || angle_deg <= a_end;
  };

  // Bounding box
  const int32_t x1 = std::max(cx - r_outer, clip.x1);
  const int32_t y1 = std::max(cy - r_outer, clip.y1);
  const int32_t x2 = std::min(cx + r_outer, clip.x2);
  const int32_t y2 = std::min(cy + r_outer, clip.y2);

  const int64_t r_outer_sq = static_cast<int64_t>(r_outer) * r_outer;
  const int64_t r_inner_sq =
      r_inner > 0 ? static_cast<int64_t>(r_inner) * r_inner : 0;

  for (int32_t row = y1; row <= y2; ++row) {
    if (row < 0 || row >= bh)
      continue;
    for (int32_t col = x1; col <= x2; ++col) {
      if (col < 0 || col >= bw)
        continue;

      const int64_t dx = static_cast<int64_t>(col - cx);
      const int64_t dy = static_cast<int64_t>(row - cy);
      const int64_t dist_sq = dx * dx + dy * dy;

      if (dist_sq > r_outer_sq || dist_sq < r_inner_sq)
        continue;

      if (!angle_in_range(col, row))
        continue;

      if (dsc.opa == 255) {
        buf[row, col] = fg;
      } else {
        buf[row, col] = detail::blend_pixel(buf[row, col], fg, dsc.opa);
      }
    }
  }
}

static_assert(Renderer<SoftwareRenderer, color::ARGB8888::pixel_type>);

}  // namespace lv
