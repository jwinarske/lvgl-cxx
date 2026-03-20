// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — widgets/table.hpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// lv::Table — grid-like table widget with per-cell text.

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

#include "../core/object.hpp"
#include "../core/types.hpp"

namespace lv {

class Table : public Object {
 public:
  explicit Table(Object* parent);
  ~Table() override;

  Table& set_cell_value(uint32_t row, uint32_t col, std::string_view text);
  Table& set_row_count(uint32_t count);
  Table& set_column_count(uint32_t count);
  Table& set_column_width(uint32_t col, int32_t width);

  [[nodiscard]] std::string_view cell_value(uint32_t row,
                                            uint32_t col) const noexcept;
  [[nodiscard]] uint32_t row_count() const noexcept;
  [[nodiscard]] uint32_t column_count() const noexcept;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace lv
