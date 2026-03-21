// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — widgets/calendar.hpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// lv::Calendar — date picker widget.

#pragma once

#include <cstdint>
#include <memory>

#include "../core/object.hpp"
#include "../core/types.hpp"

namespace lv {

class Calendar : public Object {
 public:
  explicit Calendar(Object* parent);
  ~Calendar() override;

  Calendar& set_today(CalendarDate date);
  Calendar& set_shown_date(int32_t year, int32_t month);
  Calendar& set_selected(CalendarDate date);

  [[nodiscard]] CalendarDate today() const noexcept;
  [[nodiscard]] CalendarDate shown_date() const noexcept;
  [[nodiscard]] CalendarDate selected() const noexcept;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace lv
