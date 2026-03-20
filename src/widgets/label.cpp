// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — src/widgets/label.cpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// Implementation of lv::Label.
// Like LVGL, labels auto-size to fit their text content using the
// resolved font metrics (lv_label_refr_text in lv_label.c).

#include "lvgl/widgets/label.hpp"

#include <algorithm>
#include <utility>

#include "lvgl/misc/font.hpp"

namespace lv {

struct Label::Impl {
  std::string text_;
  LabelLongMode long_mode_ = LabelLongMode::Wrap;
  bool recolor_ = false;

  // Compute text pixel dimensions using font metrics
  static void measure_text(std::string_view text,
                           const Font& font,
                           int32_t letter_spacing,
                           int32_t line_spacing,
                           int32_t& out_w,
                           int32_t& out_h) {
    const int32_t line_h = font.line_height();
    int32_t max_line_w = 0;
    int32_t cur_line_w = 0;
    int32_t num_lines = 1;

    for (std::size_t i = 0; i < text.size(); ++i) {
      const char ch = text[i];
      if (ch == '\n') {
        max_line_w = std::max(max_line_w, cur_line_w);
        cur_line_w = 0;
        ++num_lines;
        continue;
      }

      GlyphDsc g{};
      if (font.get_glyph_dsc(g, static_cast<uint32_t>(ch))) {
        cur_line_w += static_cast<int32_t>(g.advance_w);
      } else {
        // Unknown glyph — use a default advance
        cur_line_w += static_cast<int32_t>(font.line_height() / 2);
      }

      // Add letter spacing (but not after the last char on a line)
      if (i + 1 < text.size() && text[i + 1] != '\n') {
        cur_line_w += letter_spacing;
      }
    }
    max_line_w = std::max(max_line_w, cur_line_w);

    out_w = max_line_w;
    out_h = num_lines * line_h + (num_lines - 1) * line_spacing;
  }
};

Label::Label(Object* parent)
    : Object(parent), impl_(std::make_unique<Impl>()) {}

Label::~Label() = default;

Label& Label::set_text(std::string_view text) {
  impl_->text_ = text;
  // Auto-size to text content (like LVGL's lv_label_refr_text)
  int32_t tw = 0;
  int32_t th = 0;
  Impl::measure_text(impl_->text_, font_default(), 0, 0, tw, th);
  set_size(tw, th);
  invalidate();
  return *this;
}

Label& Label::set_text(std::string text) {
  impl_->text_ = std::move(text);
  int32_t tw = 0;
  int32_t th = 0;
  Impl::measure_text(impl_->text_, font_default(), 0, 0, tw, th);
  set_size(tw, th);
  invalidate();
  return *this;
}

Label& Label::set_long_mode(LabelLongMode mode) {
  impl_->long_mode_ = mode;
  invalidate();
  return *this;
}

Label& Label::set_recolor(bool en) {
  impl_->recolor_ = en;
  invalidate();
  return *this;
}

std::string_view Label::text() const noexcept {
  return impl_->text_;
}

LabelLongMode Label::long_mode() const noexcept {
  return impl_->long_mode_;
}

}  // namespace lv
