// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — widgets/spinner.hpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// lv::Spinner — rotating arc-based loading indicator.

#pragma once

#include <cstdint>
#include <memory>

#include "../core/object.hpp"
#include "../core/types.hpp"

namespace lv {

class Spinner : public Object {
 public:
  explicit Spinner(Object* parent);
  ~Spinner() override;

  Spinner& set_anim_params(uint32_t time_ms, int32_t arc_length);

  [[nodiscard]] uint32_t anim_time() const noexcept;
  [[nodiscard]] int32_t arc_length() const noexcept;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace lv
