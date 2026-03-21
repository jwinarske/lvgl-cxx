// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — src/widgets/chart.cpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// Implementation of lv::Chart.

#include "lvgl/widgets/chart.hpp"

#include <algorithm>
#include <climits>

namespace lv {

struct Chart::Impl {
  ChartType type_ = ChartType::Line;
  uint32_t point_count_ = 10;
  ChartUpdateMode update_mode_ = ChartUpdateMode::Shift;
  uint8_t h_div_ = 3;
  uint8_t v_div_ = 5;
  int32_t pri_min_ = 0;
  int32_t pri_max_ = 100;
  int32_t sec_min_ = 0;
  int32_t sec_max_ = 100;
  std::vector<std::unique_ptr<ChartSeries>> series_;
  uint32_t pressed_point_ = UINT32_MAX;
};

Chart::Chart(Object* parent)
    : Object(parent), impl_(std::make_unique<Impl>()) {}

Chart::~Chart() = default;

Chart& Chart::set_type(ChartType type) {
  impl_->type_ = type;
  invalidate();
  return *this;
}

Chart& Chart::set_point_count(uint32_t count) {
  impl_->point_count_ = count;
  for (auto& s : impl_->series_) {
    s->points.resize(count, 0);
  }
  invalidate();
  return *this;
}

Chart& Chart::set_range(ChartAxis axis, int32_t min, int32_t max) {
  if (axis == ChartAxis::Primary) {
    impl_->pri_min_ = min;
    impl_->pri_max_ = max;
  } else {
    impl_->sec_min_ = min;
    impl_->sec_max_ = max;
  }
  invalidate();
  return *this;
}

Chart& Chart::set_update_mode(ChartUpdateMode mode) {
  impl_->update_mode_ = mode;
  return *this;
}

Chart& Chart::set_div_line_count(uint8_t h_div, uint8_t v_div) {
  impl_->h_div_ = h_div;
  impl_->v_div_ = v_div;
  invalidate();
  return *this;
}

ChartSeries& Chart::add_series(Color color, ChartAxis axis) {
  auto series = std::make_unique<ChartSeries>();
  series->color = color;
  series->axis = axis;
  series->points.resize(impl_->point_count_, 0);
  auto& ref = *series;
  impl_->series_.push_back(std::move(series));
  invalidate();
  return ref;
}

Chart& Chart::remove_series(ChartSeries& series) {
  auto it = std::ranges::find_if(
      impl_->series_, [&](const auto& p) { return p.get() == &series; });
  if (it != impl_->series_.end()) {
    impl_->series_.erase(it);
    invalidate();
  }
  return *this;
}

Chart& Chart::hide_series(ChartSeries& series, bool hide) {
  series.hidden = hide;
  invalidate();
  return *this;
}

Chart& Chart::set_next_value(ChartSeries& series, int32_t value) {
  if (impl_->update_mode_ == ChartUpdateMode::Shift) {
    if (!series.points.empty()) {
      std::rotate(series.points.begin(), series.points.begin() + 1,
                  series.points.end());
      series.points.back() = value;
    }
  } else {
    // Circular: find first default slot or wrap
    series.points.push_back(value);
    if (series.points.size() > impl_->point_count_) {
      series.points.erase(series.points.begin());
    }
  }
  invalidate();
  return *this;
}

Chart& Chart::set_value_by_id(ChartSeries& series, uint32_t id, int32_t value) {
  if (id < series.points.size()) {
    series.points[id] = value;
    invalidate();
  }
  return *this;
}

Chart& Chart::refresh() {
  invalidate();
  return *this;
}

uint32_t Chart::point_count() const noexcept {
  return impl_->point_count_;
}

uint32_t Chart::pressed_point() const noexcept {
  return impl_->pressed_point_;
}

ChartType Chart::type() const noexcept {
  return impl_->type_;
}

}  // namespace lv
