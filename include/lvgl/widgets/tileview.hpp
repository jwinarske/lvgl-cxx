// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — widgets/tileview.hpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// lv::TileView — swipeable tile container widget.

#pragma once

#include <cstdint>
#include <memory>

#include "../core/object.hpp"
#include "../core/types.hpp"

namespace lv {

class TileView : public Object {
 public:
  explicit TileView(Object* parent);
  ~TileView() override;

  Object& add_tile(uint8_t col, uint8_t row, Dir scroll_dir);
  TileView& set_active(uint32_t id, AnimEnable anim = AnimEnable::Off);

  [[nodiscard]] uint32_t active_id() const noexcept;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace lv
