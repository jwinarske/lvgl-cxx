// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — src/widgets/span.cpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// Implementation of lv::Span.

#include "lvgl/widgets/span.hpp"

#include <vector>

namespace lv {

struct Span::Impl {
  std::vector<SpanDescriptor> spans_;
  int32_t max_lines_ = -1;
  LabelLongMode overflow_ = LabelLongMode::Wrap;
};

Span::Span(Object* parent) : Object(parent), impl_(std::make_unique<Impl>()) {}

Span::~Span() = default;

SpanDescriptor& Span::new_span() {
  impl_->spans_.emplace_back();
  invalidate();
  return impl_->spans_.back();
}

Span& Span::set_max_lines(int32_t lines) {
  impl_->max_lines_ = lines;
  invalidate();
  return *this;
}

Span& Span::set_overflow(LabelLongMode mode) {
  impl_->overflow_ = mode;
  invalidate();
  return *this;
}

std::size_t Span::span_count() const noexcept {
  return impl_->spans_.size();
}

int32_t Span::max_lines() const noexcept {
  return impl_->max_lines_;
}

}  // namespace lv
