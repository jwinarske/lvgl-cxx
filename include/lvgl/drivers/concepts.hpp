// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — drivers/concepts.hpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// Concept definitions for platform display and input device drivers.

#pragma once

#include <concepts>
#include <cstdint>

#include "../core/display.hpp"
#include "../misc/area.hpp"

namespace lv::drivers {

// ── DisplayDriver concept ───────────────────────────────────────────────────
// A DisplayDriver must provide init/deinit and a flush method.
template <typename D>
concept DisplayDriver = requires(D drv, Display& disp, const Area& area) {
  { drv.init() } -> std::same_as<void>;
  { drv.deinit() } -> std::same_as<void>;
  { drv.width() } -> std::convertible_to<int32_t>;
  { drv.height() } -> std::convertible_to<int32_t>;
};

// ── InputState ──────────────────────────────────────────────────────────────
// Polled state from an input device, passed to the resolver.
struct InputState {
  enum class Type : uint8_t { None, Pointer, Keypad, Encoder };

  Type type = Type::None;
  bool pressed = false;
  int32_t x = 0;
  int32_t y = 0;
  uint32_t key = 0;
  int32_t encoder_diff = 0;
};

// ── InputDriver concept ─────────────────────────────────────────────────────
template <typename D>
concept InputDriver = requires(D drv, InputState& state) {
  { drv.read(state) } -> std::same_as<void>;
};

}  // namespace lv::drivers
