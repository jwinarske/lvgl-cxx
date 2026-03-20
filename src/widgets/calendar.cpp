// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — src/widgets/calendar.cpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// Implementation of lv::Calendar.

#include "lvgl/widgets/calendar.hpp"

namespace lv {

struct Calendar::Impl {
  CalendarDate today_{2026, 1, 1};
  CalendarDate shown_{2026, 1, 1};
  CalendarDate selected_{2026, 1, 1};
};

Calendar::Calendar(Object* parent)
    : Object(parent), impl_(std::make_unique<Impl>()) {}

Calendar::~Calendar() = default;

Calendar& Calendar::set_today(CalendarDate date) {
  impl_->today_ = date;
  invalidate();
  return *this;
}

Calendar& Calendar::set_shown_date(int32_t year, int32_t month) {
  impl_->shown_.year = year;
  impl_->shown_.month = month;
  invalidate();
  return *this;
}

Calendar& Calendar::set_selected(CalendarDate date) {
  impl_->selected_ = date;
  invalidate();
  return *this;
}

CalendarDate Calendar::today() const noexcept {
  return impl_->today_;
}

CalendarDate Calendar::shown_date() const noexcept {
  return impl_->shown_;
}

CalendarDate Calendar::selected() const noexcept {
  return impl_->selected_;
}

}  // namespace lv
