// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — widgets/buttonmatrix.hpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// lv::ButtonMatrix — grid of text buttons defined by a string map.

#pragma once

#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "../core/object.hpp"
#include "../core/types.hpp"

namespace lv {

class ButtonMatrix : public Object {
 public:
  explicit ButtonMatrix(Object* parent);
  ~ButtonMatrix() override;

  ButtonMatrix& set_map(std::span<const std::string_view> map);
  ButtonMatrix& set_ctrl_map(std::span<const ButtonCtrl> ctrl_map);
  ButtonMatrix& set_selected_btn(uint32_t idx);
  ButtonMatrix& set_btn_ctrl(uint32_t idx, ButtonCtrl ctrl);
  ButtonMatrix& clear_btn_ctrl(uint32_t idx, ButtonCtrl ctrl);
  ButtonMatrix& set_one_checked(bool en);

  [[nodiscard]] uint32_t selected_btn() const noexcept;
  [[nodiscard]] std::string_view btn_text(uint32_t idx) const noexcept;
  [[nodiscard]] uint32_t btn_count() const noexcept;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace lv
