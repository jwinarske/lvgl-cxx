// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — src/widgets/tileview.cpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// Implementation of lv::TileView.

#include "lvgl/widgets/tileview.hpp"
#include "lvgl/widgets/container.hpp"

namespace lv {

struct TileView::Impl {
  uint32_t active_id_ = 0;
};

TileView::TileView(Object* parent)
    : Object(parent), impl_(std::make_unique<Impl>()) {
  add_flag(ObjFlags::Scrollable);
}

TileView::~TileView() = default;

Object& TileView::add_tile(uint8_t /*col*/,
                           uint8_t /*row*/,
                           Dir /*scroll_dir*/) {
  auto& tile = create<Container>();
  return tile;
}

TileView& TileView::set_active(uint32_t id, AnimEnable /*anim*/) {
  impl_->active_id_ = id;
  invalidate();
  return *this;
}

uint32_t TileView::active_id() const noexcept {
  return impl_->active_id_;
}

}  // namespace lv
