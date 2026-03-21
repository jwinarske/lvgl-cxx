// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Joel Winarske
//
// Phase 4 unit tests: SoftwareRenderer

#include <gtest/gtest.h>

#include "lvgl/core/draw_buffer.hpp"
#include "lvgl/draw/software_renderer.hpp"

namespace {

using Pixel = lv::color::ARGB8888::pixel_type;

TEST(SoftwareRenderer, SatisfiesRendererConcept) {
  static_assert(lv::Renderer<lv::SoftwareRenderer, Pixel>);
}

TEST(SoftwareRenderer, RenderRectFillsPixels) {
  auto buf = lv::DrawBuffer<Pixel>::allocate(10, 10);
  buf.fill(Pixel{0, 0, 0, 255});

  lv::RectTask task;
  task.clip_area = {0, 0, 9, 9};
  task.bounds = {2, 2, 7, 7};
  task.dsc.bg_color = lv::Color::Red();
  task.dsc.bg_opa = 255;
  task.dsc.radius = 0;

  std::vector<lv::DrawTask> tasks;
  tasks.emplace_back(task);

  lv::SoftwareRenderer renderer;
  renderer.execute(buf, tasks);

  // Pixel inside the rect should be red
  EXPECT_EQ((buf[3, 4]).r, 255);
  EXPECT_EQ((buf[3, 4]).g, 0);
  EXPECT_EQ((buf[3, 4]).b, 0);

  // Pixel outside the rect should be black (untouched)
  EXPECT_EQ((buf[0, 0]).r, 0);
  EXPECT_EQ((buf[0, 0]).g, 0);
  EXPECT_EQ((buf[0, 0]).b, 0);
}

TEST(SoftwareRenderer, RenderRectClipsToArea) {
  auto buf = lv::DrawBuffer<Pixel>::allocate(10, 10);
  buf.fill(Pixel{0, 0, 0, 255});

  lv::RectTask task;
  // Clip area is only top-left quadrant
  task.clip_area = {0, 0, 4, 4};
  // But rect covers full buffer
  task.bounds = {0, 0, 9, 9};
  task.dsc.bg_color = lv::Color::Green();
  task.dsc.bg_opa = 255;
  task.dsc.radius = 0;

  std::vector<lv::DrawTask> tasks;
  tasks.emplace_back(task);

  lv::SoftwareRenderer renderer;
  renderer.execute(buf, tasks);

  // Inside clip area: green
  EXPECT_EQ((buf[2, 2]).g, 255);
  EXPECT_EQ((buf[2, 2]).r, 0);

  // Outside clip area: still black
  EXPECT_EQ((buf[7, 7]).g, 0);
}

TEST(SoftwareRenderer, RenderRectWithBorder) {
  auto buf = lv::DrawBuffer<Pixel>::allocate(20, 20);
  buf.fill(Pixel{0, 0, 0, 255});

  lv::RectTask task;
  task.clip_area = {0, 0, 19, 19};
  task.bounds = {2, 2, 17, 17};
  task.dsc.bg_color = lv::Color::White();
  task.dsc.bg_opa = 255;
  task.dsc.border_color = lv::Color::Red();
  task.dsc.border_width = 2;
  task.dsc.border_opa = 255;
  task.dsc.radius = 0;

  std::vector<lv::DrawTask> tasks;
  tasks.emplace_back(task);

  lv::SoftwareRenderer renderer;
  renderer.execute(buf, tasks);

  // Border pixel (row=2, col=2 — top-left corner)
  EXPECT_EQ((buf[2, 2]).r, 255);
  EXPECT_EQ((buf[2, 2]).g, 0);

  // Interior pixel (row=5, col=5)
  EXPECT_EQ((buf[5, 5]).r, 255);
  EXPECT_EQ((buf[5, 5]).g, 255);
  EXPECT_EQ((buf[5, 5]).b, 255);
}

TEST(SoftwareRenderer, RenderRoundedRectCutsCorners) {
  auto buf = lv::DrawBuffer<Pixel>::allocate(20, 20);
  buf.fill(Pixel{0, 0, 0, 255});

  lv::RectTask task;
  task.clip_area = {0, 0, 19, 19};
  task.bounds = {0, 0, 19, 19};
  task.dsc.bg_color = lv::Color::Blue();
  task.dsc.bg_opa = 255;
  task.dsc.radius = 5;

  std::vector<lv::DrawTask> tasks;
  tasks.emplace_back(task);

  lv::SoftwareRenderer renderer;
  renderer.execute(buf, tasks);

  // Corner pixel (0,0) should NOT be filled (rounded off)
  EXPECT_EQ((buf[0, 0]).b, 0);

  // Center pixel should be filled
  EXPECT_EQ((buf[10, 10]).b, 255);

  // A pixel just inside the radius should be filled
  EXPECT_EQ((buf[5, 5]).b, 255);
}

TEST(SoftwareRenderer, RenderLine) {
  auto buf = lv::DrawBuffer<Pixel>::allocate(10, 10);
  buf.fill(Pixel{0, 0, 0, 255});

  lv::LineTask task;
  task.clip_area = {0, 0, 9, 9};
  task.p1 = {0, 0};
  task.p2 = {9, 9};
  task.dsc.color = lv::Color::White();
  task.dsc.opa = 255;

  std::vector<lv::DrawTask> tasks;
  tasks.emplace_back(task);

  lv::SoftwareRenderer renderer;
  renderer.execute(buf, tasks);

  // Diagonal pixels should be white
  EXPECT_EQ((buf[0, 0]).r, 255);
  EXPECT_EQ((buf[5, 5]).r, 255);
  EXPECT_EQ((buf[9, 9]).r, 255);

  // Off-diagonal pixel should be black
  EXPECT_EQ((buf[0, 5]).r, 0);
}

TEST(SoftwareRenderer, EmptyTaskListIsNoOp) {
  auto buf = lv::DrawBuffer<Pixel>::allocate(5, 5);
  Pixel fill{42, 42, 42, 255};
  buf.fill(fill);

  lv::SoftwareRenderer renderer;
  renderer.execute(buf, std::span<const lv::DrawTask>{});

  // Buffer should be unchanged
  EXPECT_EQ((buf[2, 2]).r, 42);
}

TEST(SoftwareRenderer, MultipleTasksRendered) {
  auto buf = lv::DrawBuffer<Pixel>::allocate(20, 20);
  buf.fill(Pixel{0, 0, 0, 255});

  lv::RectTask r1;
  r1.clip_area = {0, 0, 19, 19};
  r1.bounds = {0, 0, 9, 9};
  r1.dsc.bg_color = lv::Color::Red();
  r1.dsc.bg_opa = 255;

  lv::RectTask r2;
  r2.clip_area = {0, 0, 19, 19};
  r2.bounds = {10, 10, 19, 19};
  r2.dsc.bg_color = lv::Color::Blue();
  r2.dsc.bg_opa = 255;

  std::vector<lv::DrawTask> tasks;
  tasks.emplace_back(r1);
  tasks.emplace_back(r2);

  lv::SoftwareRenderer renderer;
  renderer.execute(buf, tasks);

  EXPECT_EQ((buf[5, 5]).r, 255);  // Red rect
  EXPECT_EQ((buf[5, 5]).b, 0);
  EXPECT_EQ((buf[15, 15]).b, 255);  // Blue rect
  EXPECT_EQ((buf[15, 15]).r, 0);
}

}  // namespace
