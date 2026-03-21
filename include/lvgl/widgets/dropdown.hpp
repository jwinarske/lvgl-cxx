// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — widgets/dropdown.hpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// lv::Dropdown — selectable list that opens/closes on click.

#pragma once

#include <memory>
#include <string>
#include <string_view>

#include "../core/object.hpp"
#include "../core/types.hpp"

namespace lv {

class Dropdown : public Object {
 public:
  explicit Dropdown(Object* parent);
  ~Dropdown() override;

  Dropdown& set_options(std::string_view options);
  Dropdown& set_selected(uint32_t index);
  Dropdown& set_dir(Dir dir);
  Dropdown& set_text(std::string_view text);
  Dropdown& set_symbol(std::string_view symbol);

  [[nodiscard]] uint32_t selected() const noexcept;
  [[nodiscard]] std::string selected_str() const;
  [[nodiscard]] uint32_t option_count() const noexcept;

  void open();
  void close();
  [[nodiscard]] bool is_open() const noexcept;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace lv
