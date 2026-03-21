// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — misc/area.hpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0

#pragma once

#include <algorithm>
#include <cstdint>

namespace lv {

struct Point {
  int32_t x = 0;
  int32_t y = 0;

  constexpr Point() noexcept = default;
  constexpr Point(int32_t px, int32_t py) noexcept : x(px), y(py) {}

  auto operator<=>(const Point&) const = default;

  [[nodiscard]] constexpr Point operator+(Point o) const noexcept {
    return {x + o.x, y + o.y};
  }
  [[nodiscard]] constexpr Point operator-(Point o) const noexcept {
    return {x - o.x, y - o.y};
  }
};

struct Area {
  int32_t x1 = 0, y1 = 0, x2 = 0, y2 = 0;

  constexpr Area() noexcept = default;
  constexpr Area(int32_t ax1, int32_t ay1, int32_t ax2, int32_t ay2) noexcept
      : x1(ax1), y1(ay1), x2(ax2), y2(ay2) {}

  // Construct from position + size
  [[nodiscard]] static constexpr Area from_size(int32_t x,
                                                int32_t y,
                                                int32_t w,
                                                int32_t h) noexcept {
    return {x, y, x + w - 1, y + h - 1};
  }

  [[nodiscard]] constexpr int32_t width() const noexcept { return x2 - x1 + 1; }
  [[nodiscard]] constexpr int32_t height() const noexcept {
    return y2 - y1 + 1;
  }

  [[nodiscard]] constexpr bool contains(Point p) const noexcept {
    return p.x >= x1 && p.x <= x2 && p.y >= y1 && p.y <= y2;
  }
  [[nodiscard]] constexpr bool intersects(const Area& o) const noexcept {
    return x1 <= o.x2 && x2 >= o.x1 && y1 <= o.y2 && y2 >= o.y1;
  }
  [[nodiscard]] constexpr Area intersect(const Area& o) const noexcept {
    return {std::max(x1, o.x1), std::max(y1, o.y1), std::min(x2, o.x2),
            std::min(y2, o.y2)};
  }
  [[nodiscard]] constexpr Area unite(const Area& o) const noexcept {
    return {std::min(x1, o.x1), std::min(y1, o.y1), std::max(x2, o.x2),
            std::max(y2, o.y2)};
  }
  [[nodiscard]] constexpr bool empty() const noexcept {
    return x2 < x1 || y2 < y1;
  }

  auto operator<=>(const Area&) const = default;
};

}  // namespace lv
