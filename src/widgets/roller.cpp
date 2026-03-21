// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — src/widgets/roller.cpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// Implementation of lv::Roller.

#include "lvgl/widgets/roller.hpp"

#include <algorithm>

namespace lv {

struct Roller::Impl {
  std::string options_;
  uint32_t selected_ = 0;
  RollerMode mode_ = RollerMode::Normal;
  uint32_t visible_row_count_ = 3;
};

Roller::Roller(Object* parent)
    : Object(parent), impl_(std::make_unique<Impl>()) {}

Roller::~Roller() = default;

Roller& Roller::set_options(std::string_view options, RollerMode mode) {
  impl_->options_ = options;
  impl_->mode_ = mode;
  impl_->selected_ = 0;
  invalidate();
  return *this;
}

Roller& Roller::set_selected(uint32_t index, AnimEnable /*anim*/) {
  impl_->selected_ = index;
  invalidate();
  return *this;
}

Roller& Roller::set_visible_row_count(uint32_t count) {
  impl_->visible_row_count_ = count;
  invalidate();
  return *this;
}

uint32_t Roller::selected() const noexcept {
  return impl_->selected_;
}

std::string Roller::selected_str() const {
  const auto& opts = impl_->options_;
  if (opts.empty())
    return {};
  uint32_t idx = 0;
  std::size_t start = 0;
  while (start < opts.size()) {
    auto end = opts.find('\n', start);
    if (end == std::string::npos)
      end = opts.size();
    if (idx == impl_->selected_)
      return opts.substr(start, end - start);
    ++idx;
    start = end + 1;
  }
  return {};
}

uint32_t Roller::option_count() const noexcept {
  if (impl_->options_.empty())
    return 0;
  return static_cast<uint32_t>(
      std::count(impl_->options_.begin(), impl_->options_.end(), '\n') + 1);
}

}  // namespace lv
