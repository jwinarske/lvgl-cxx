// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — src/widgets/table.cpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// Implementation of lv::Table.

#include "lvgl/widgets/table.hpp"

#include <vector>

namespace lv {

struct Table::Impl {
  std::vector<std::vector<std::string>> cells_;
  uint32_t row_count_ = 0;
  uint32_t col_count_ = 0;
  std::vector<int32_t> col_widths_;
};

Table::Table(Object* parent)
    : Object(parent), impl_(std::make_unique<Impl>()) {}

Table::~Table() = default;

Table& Table::set_cell_value(uint32_t row,
                             uint32_t col,
                             std::string_view text) {
  if (row < impl_->row_count_ && col < impl_->col_count_) {
    impl_->cells_[row][col] = text;
    invalidate();
  }
  return *this;
}

Table& Table::set_row_count(uint32_t count) {
  impl_->row_count_ = count;
  impl_->cells_.resize(count);
  for (auto& row : impl_->cells_) {
    row.resize(impl_->col_count_);
  }
  invalidate();
  return *this;
}

Table& Table::set_column_count(uint32_t count) {
  impl_->col_count_ = count;
  for (auto& row : impl_->cells_) {
    row.resize(count);
  }
  impl_->col_widths_.resize(count, 80);
  invalidate();
  return *this;
}

Table& Table::set_column_width(uint32_t col, int32_t width) {
  if (col < impl_->col_count_) {
    impl_->col_widths_[col] = width;
    invalidate();
  }
  return *this;
}

std::string_view Table::cell_value(uint32_t row, uint32_t col) const noexcept {
  if (row < impl_->row_count_ && col < impl_->col_count_) {
    return impl_->cells_[row][col];
  }
  return {};
}

uint32_t Table::row_count() const noexcept {
  return impl_->row_count_;
}

uint32_t Table::column_count() const noexcept {
  return impl_->col_count_;
}

}  // namespace lv
