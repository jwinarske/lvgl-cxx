// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — core/layer.hpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// Layer — intermediate render target that collects draw tasks.
// Each Object::on_draw() pushes typed draw tasks into a Layer.
// The renderer then processes the task queue.

#pragma once

#include <array>
#include <span>
#include <vector>

#include "../draw/descriptors.hpp"
#include "../misc/area.hpp"

namespace lv {

class Layer {
 public:
  Layer() noexcept = default;
  explicit Layer(Area clip) noexcept : clip_area_(clip) {}

  void draw_rect(Area bounds, RectDescriptor dsc) {
    RectTask task;
    task.clip_area = clip_area_;
    task.bounds = bounds;
    task.dsc = dsc;
    tasks_.emplace_back(task);
  }

  void draw_label(Point pos, LabelDescriptor dsc) {
    LabelTask task;
    task.clip_area = clip_area_;
    task.pos = pos;
    task.dsc = dsc;
    tasks_.emplace_back(task);
  }

  void draw_image(Area bounds, ImageDescriptor dsc) {
    ImageTask task;
    task.clip_area = clip_area_;
    task.bounds = bounds;
    task.dsc = dsc;
    tasks_.emplace_back(task);
  }

  void draw_line(Point p1, Point p2, LineDescriptor dsc) {
    LineTask task;
    task.clip_area = clip_area_;
    task.p1 = p1;
    task.p2 = p2;
    task.dsc = dsc;
    tasks_.emplace_back(task);
  }

  void draw_arc(Point center, ArcDescriptor dsc) {
    ArcTask task;
    task.clip_area = clip_area_;
    task.center = center;
    task.dsc = dsc;
    tasks_.emplace_back(task);
  }

  void draw_triangle(std::array<Point, 3> pts, TriangleDescriptor dsc) {
    TriangleTask task;
    task.clip_area = clip_area_;
    task.pts = pts;
    task.dsc = dsc;
    tasks_.emplace_back(task);
  }

  void set_clip_area(Area a) noexcept { clip_area_ = a; }
  [[nodiscard]] Area clip_area() const noexcept { return clip_area_; }

  [[nodiscard]] std::span<const DrawTask> tasks() const noexcept {
    return tasks_;
  }

  void clear() noexcept { tasks_.clear(); }

 private:
  Area clip_area_;
  std::vector<DrawTask> tasks_;
};

}  // namespace lv
