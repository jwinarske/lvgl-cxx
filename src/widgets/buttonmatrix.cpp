// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — src/widgets/buttonmatrix.cpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// Implementation of lv::ButtonMatrix.

#include "lvgl/widgets/buttonmatrix.hpp"

namespace lv {

struct ButtonMatrix::Impl {
  std::vector<std::string> map_;
  std::vector<ButtonCtrl> ctrl_map_;
  uint32_t selected_ = UINT32_MAX;
  bool one_checked_ = false;
};

ButtonMatrix::ButtonMatrix(Object* parent)
    : Object(parent), impl_(std::make_unique<Impl>()) {
  add_flag(ObjFlags::Clickable);
}

ButtonMatrix::~ButtonMatrix() = default;

ButtonMatrix& ButtonMatrix::set_map(std::span<const std::string_view> map) {
  impl_->map_.clear();
  impl_->map_.reserve(map.size());
  for (auto sv : map) {
    impl_->map_.emplace_back(sv);
  }
  impl_->ctrl_map_.resize(impl_->map_.size(), ButtonCtrl::None);
  invalidate();
  return *this;
}

ButtonMatrix& ButtonMatrix::set_ctrl_map(std::span<const ButtonCtrl> ctrl_map) {
  impl_->ctrl_map_.assign(ctrl_map.begin(), ctrl_map.end());
  invalidate();
  return *this;
}

ButtonMatrix& ButtonMatrix::set_selected_btn(uint32_t idx) {
  impl_->selected_ = idx;
  invalidate();
  return *this;
}

ButtonMatrix& ButtonMatrix::set_btn_ctrl(uint32_t idx, ButtonCtrl ctrl) {
  if (idx < impl_->ctrl_map_.size()) {
    impl_->ctrl_map_[idx] =
        static_cast<ButtonCtrl>(static_cast<uint16_t>(impl_->ctrl_map_[idx]) |
                                static_cast<uint16_t>(ctrl));
    invalidate();
  }
  return *this;
}

ButtonMatrix& ButtonMatrix::clear_btn_ctrl(uint32_t idx, ButtonCtrl ctrl) {
  if (idx < impl_->ctrl_map_.size()) {
    impl_->ctrl_map_[idx] =
        static_cast<ButtonCtrl>(static_cast<uint16_t>(impl_->ctrl_map_[idx]) &
                                ~static_cast<uint16_t>(ctrl));
    invalidate();
  }
  return *this;
}

ButtonMatrix& ButtonMatrix::set_one_checked(bool en) {
  impl_->one_checked_ = en;
  return *this;
}

uint32_t ButtonMatrix::selected_btn() const noexcept {
  return impl_->selected_;
}

std::string_view ButtonMatrix::btn_text(uint32_t idx) const noexcept {
  if (idx < impl_->map_.size()) {
    return impl_->map_[idx];
  }
  return {};
}

uint32_t ButtonMatrix::btn_count() const noexcept {
  return static_cast<uint32_t>(impl_->map_.size());
}

}  // namespace lv
