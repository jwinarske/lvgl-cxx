// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — src/widgets/dropdown.cpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// Implementation of lv::Dropdown.

#include "lvgl/widgets/dropdown.hpp"

#include <algorithm>
#include <sstream>

namespace lv {

struct Dropdown::Impl {
  std::string options_;
  uint32_t selected_ = 0;
  Dir dir_ = Dir::Bottom;
  std::string text_;
  std::string symbol_;
  bool open_ = false;
};

Dropdown::Dropdown(Object* parent)
    : Object(parent), impl_(std::make_unique<Impl>()) {
  add_flag(ObjFlags::Clickable);
}

Dropdown::~Dropdown() = default;

Dropdown& Dropdown::set_options(std::string_view options) {
  impl_->options_ = options;
  impl_->selected_ = 0;
  invalidate();
  return *this;
}

Dropdown& Dropdown::set_selected(uint32_t index) {
  impl_->selected_ = index;
  invalidate();
  return *this;
}

Dropdown& Dropdown::set_dir(Dir dir) {
  impl_->dir_ = dir;
  invalidate();
  return *this;
}

Dropdown& Dropdown::set_text(std::string_view text) {
  impl_->text_ = text;
  invalidate();
  return *this;
}

Dropdown& Dropdown::set_symbol(std::string_view symbol) {
  impl_->symbol_ = symbol;
  invalidate();
  return *this;
}

uint32_t Dropdown::selected() const noexcept {
  return impl_->selected_;
}

std::string Dropdown::selected_str() const {
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

uint32_t Dropdown::option_count() const noexcept {
  if (impl_->options_.empty())
    return 0;
  return static_cast<uint32_t>(
      std::count(impl_->options_.begin(), impl_->options_.end(), '\n') + 1);
}

void Dropdown::open() {
  impl_->open_ = true;
  invalidate();
}

void Dropdown::close() {
  impl_->open_ = false;
  invalidate();
}

bool Dropdown::is_open() const noexcept {
  return impl_->open_;
}

}  // namespace lv
