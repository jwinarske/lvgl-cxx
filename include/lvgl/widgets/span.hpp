// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — widgets/span.hpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// lv::Span — rich-text span group widget.

#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "../core/object.hpp"
#include "../core/types.hpp"
#include "../misc/color.hpp"

namespace lv {

class Font;

struct SpanDescriptor {
  std::string text;
  Color color = Color::White();
  const Font* font = nullptr;
};

class Span : public Object {
 public:
  explicit Span(Object* parent);
  ~Span() override;

  SpanDescriptor& new_span();
  Span& set_max_lines(int32_t lines);
  Span& set_overflow(LabelLongMode mode);

  [[nodiscard]] std::size_t span_count() const noexcept;
  [[nodiscard]] int32_t max_lines() const noexcept;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace lv
