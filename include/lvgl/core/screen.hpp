// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — core/screen.hpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// lv::Screen — root node of a display's widget tree.
//
// Corresponds to an lv_obj_t with no parent in LVGL v9.  Each Display owns
// one or more Screens; exactly one is the "active" screen at any time.
// For unit-testing, a Screen may be constructed with display == nullptr.

#pragma once

#include "object.hpp"

namespace lv {

class Display;

class Screen : public Object {
 public:
  /// Construct a screen attached to @p display.
  /// @p display may be nullptr in unit tests (no rendering).
  explicit Screen(Display* display = nullptr);
  ~Screen() override;

  // Non-copyable, non-movable — inherited from Object.

  /// The Display that owns this screen or nullptr if unattached.
  [[nodiscard]] Display* owner_display() noexcept;
  [[nodiscard]] const Display* owner_display() const noexcept;

 protected:
  void on_create() override;

 private:
  Display* display_ = nullptr;
};

}  // namespace lv
