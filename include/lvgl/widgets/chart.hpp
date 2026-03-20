// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — widgets/chart.hpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// lv::Chart — data-visualization chart widget with line, bar and scatter modes.

#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "../core/object.hpp"
#include "../core/types.hpp"
#include "../misc/color.hpp"

namespace lv {

struct ChartSeries {
  Color color;
  ChartAxis axis = ChartAxis::Primary;
  std::vector<int32_t> points;
  bool hidden = false;
};

class Chart : public Object {
 public:
  explicit Chart(Object* parent);
  ~Chart() override;

  Chart& set_type(ChartType type);
  Chart& set_point_count(uint32_t count);
  Chart& set_range(ChartAxis axis, int32_t min, int32_t max);
  Chart& set_update_mode(ChartUpdateMode mode);
  Chart& set_div_line_count(uint8_t h_div, uint8_t v_div);

  ChartSeries& add_series(Color color, ChartAxis axis);
  Chart& remove_series(ChartSeries& series);
  Chart& hide_series(ChartSeries& series, bool hide);
  Chart& set_next_value(ChartSeries& series, int32_t value);
  Chart& set_value_by_id(ChartSeries& series, uint32_t id, int32_t value);
  Chart& refresh();

  [[nodiscard]] uint32_t point_count() const noexcept;
  [[nodiscard]] uint32_t pressed_point() const noexcept;
  [[nodiscard]] ChartType type() const noexcept;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace lv
