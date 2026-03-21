// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — draw/descriptors.hpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// Draw descriptor structs and DrawTask variant.
// Mirrors lv_draw_rect_dsc_t, lv_draw_label_dsc_t, etc. from LVGL v9.5.0.

#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <variant>

#include "../misc/area.hpp"
#include "../misc/color.hpp"

namespace lv {

// Forward declarations
class Font;

// ── Draw descriptors ────────────────────────────────────────────────────────

struct RectDescriptor {
  Color bg_color;
  uint8_t bg_opa = 255;
  ColorFilter bg_grad;
  int32_t radius = 0;
  Color border_color;
  int32_t border_width = 0;
  uint8_t border_opa = 255;
  int32_t border_side = 0xF;  // all sides
  Color outline_color;
  int32_t outline_width = 0;
  int32_t outline_pad = 0;
  uint8_t outline_opa = 255;
  Color shadow_color;
  int32_t shadow_width = 0;
  int32_t shadow_offset_x = 0;
  int32_t shadow_offset_y = 0;
  int32_t shadow_spread = 0;
  uint8_t shadow_opa = 255;
};

struct LabelDescriptor {
  std::string text;
  const Font* font = nullptr;
  Color color;
  uint8_t opa = 255;
  int32_t letter_spacing = 0;
  int32_t line_spacing = 0;
  int32_t text_align = 0;  // 0=left, 1=center, 2=right
};

struct ImageDescriptor {
  const void* src = nullptr;
  uint8_t opa = 255;
  Color recolor;
  uint8_t recolor_opa = 0;
  int32_t rotation = 0;   // 0.1 degree units
  int32_t scale_x = 256;  // 256 = 100%
  int32_t scale_y = 256;
  int32_t pivot_x = 0;
  int32_t pivot_y = 0;
};

struct LineDescriptor {
  Color color;
  int32_t width = 1;
  uint8_t opa = 255;
  bool rounded = false;
};

struct ArcDescriptor {
  Color color;
  int32_t width = 1;
  int32_t radius = 0;
  int32_t start_angle = 0;
  int32_t end_angle = 360;
  uint8_t opa = 255;
  bool rounded = false;
};

struct TriangleDescriptor {
  Color color;
  uint8_t opa = 255;
};

struct VectorDescriptor {
  // Placeholder for SVG-style vector path rendering
};

struct MaskDescriptor {
  // Placeholder for clipping mask operations
};

// ── Draw task base + typed tasks ────────────────────────────────────────────

struct DrawTaskBase {
  Area clip_area;
  int32_t layer_id = 0;
};

struct RectTask : DrawTaskBase {
  Area bounds;
  RectDescriptor dsc;
};

struct LabelTask : DrawTaskBase {
  Point pos;
  LabelDescriptor dsc;
};

struct ImageTask : DrawTaskBase {
  Area bounds;
  ImageDescriptor dsc;
};

struct LineTask : DrawTaskBase {
  Point p1;
  Point p2;
  LineDescriptor dsc;
};

struct ArcTask : DrawTaskBase {
  Point center;
  ArcDescriptor dsc;
};

struct TriangleTask : DrawTaskBase {
  std::array<Point, 3> pts;
  TriangleDescriptor dsc;
};

struct VectorTask : DrawTaskBase {
  VectorDescriptor dsc;
};

struct MaskTask : DrawTaskBase {
  MaskDescriptor dsc;
};

using DrawTask = std::variant<RectTask,
                              LabelTask,
                              ImageTask,
                              LineTask,
                              ArcTask,
                              TriangleTask,
                              VectorTask,
                              MaskTask>;

}  // namespace lv