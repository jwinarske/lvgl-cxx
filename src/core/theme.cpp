// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — src/core/theme.cpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// Built-in themes: DefaultTheme (material blue) and MonoTheme (black/white).
// Style properties mirror lv_theme_default.c / lv_theme_mono.c from LVGL
// v9.5.0.

#include "lvgl/core/theme.hpp"
#include "lvgl/core/object.hpp"

namespace lv {

// ── Theme base defaults
// ───────────────────────────────────────────────────────

Color Theme::primary_color() const noexcept {
  return Color::Blue();
}
Color Theme::secondary_color() const noexcept {
  return Color::White();
}
Color Theme::fg_color() const noexcept {
  return Color::Black();
}
Color Theme::bg_color() const noexcept {
  return Color::White();
}

// ── DefaultTheme
// ──────────────────────────────────────────────────────────────

DefaultTheme::DefaultTheme(Color primary, Color secondary) noexcept
    : primary_(primary), secondary_(secondary) {
  // ── Base style (applied to every object) ─────────────────────────────────
  style_base_.set(prop::BgColor{}, Color::White())
      .set(prop::BgOpacity{}, 255)
      .set(prop::BorderColor{}, Color::from_hex(0xBDBDBD))  // gray-400
      .set(prop::BorderWidth{}, 1)
      .set(prop::BorderOpacity{}, 255)
      .set(prop::Radius{}, 6)
      .set(prop::PadTop{}, 4)
      .set(prop::PadBottom{}, 4)
      .set(prop::PadLeft{}, 8)
      .set(prop::PadRight{}, 8)
      .set(prop::TextColor{}, Color::Black())
      .set(prop::TextOpacity{}, 255);

  // ── Pressed state style ───────────────────────────────────────────────────
  style_pressed_
      .set(prop::BgColor{}, primary_.mix(Color::Black(), 51))  // ~80% primary
      .set(prop::BgOpacity{}, 255);

  // ── Focused state style ───────────────────────────────────────────────────
  style_focused_.set(prop::OutlineColor{}, primary_)
      .set(prop::OutlineWidth{}, 3)
      .set(prop::OutlineOpacity{}, 255)
      .set(prop::OutlinePad{}, 2);
}

void DefaultTheme::apply(Object& obj) {
  obj.add_style(style_base_, StyleSelector::Default);
  obj.add_style(style_pressed_, StyleSelector::Pressed);
  obj.add_style(style_focused_, StyleSelector::Focused);
}

Color DefaultTheme::primary_color() const noexcept {
  return primary_;
}
Color DefaultTheme::secondary_color() const noexcept {
  return secondary_;
}
Color DefaultTheme::fg_color() const noexcept {
  return Color::Black();
}
Color DefaultTheme::bg_color() const noexcept {
  return Color::White();
}

// ── MonoTheme
// ─────────────────────────────────────────────────────────────────

MonoTheme::MonoTheme() noexcept {
  // ── Base style ────────────────────────────────────────────────────────────
  style_base_.set(prop::BgColor{}, Color::White())
      .set(prop::BgOpacity{}, 255)
      .set(prop::BorderColor{}, Color::Black())
      .set(prop::BorderWidth{}, 1)
      .set(prop::BorderOpacity{}, 255)
      .set(prop::Radius{}, 0)  // no rounding for mono
      .set(prop::PadTop{}, 2)
      .set(prop::PadBottom{}, 2)
      .set(prop::PadLeft{}, 4)
      .set(prop::PadRight{}, 4)
      .set(prop::TextColor{}, Color::Black())
      .set(prop::TextOpacity{}, 255);

  // ── Pressed (inverted) ────────────────────────────────────────────────────
  style_pressed_.set(prop::BgColor{}, Color::Black())
      .set(prop::BgOpacity{}, 255)
      .set(prop::TextColor{}, Color::White())
      .set(prop::TextOpacity{}, 255);

  // ── Focused (dashed border approximated with wider border) ────────────────
  style_focused_.set(prop::BorderColor{}, Color::Black())
      .set(prop::BorderWidth{}, 3)
      .set(prop::BorderOpacity{}, 255);
}

void MonoTheme::apply(Object& obj) {
  obj.add_style(style_base_, StyleSelector::Default);
  obj.add_style(style_pressed_, StyleSelector::Pressed);
  obj.add_style(style_focused_, StyleSelector::Focused);
}

Color MonoTheme::primary_color() const noexcept {
  return Color::Black();
}
Color MonoTheme::secondary_color() const noexcept {
  return Color::White();
}
Color MonoTheme::fg_color() const noexcept {
  return Color::Black();
}
Color MonoTheme::bg_color() const noexcept {
  return Color::White();
}

}  // namespace lv
