// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Joel Winarske
//
// Phase 4 unit tests: Layer and draw descriptors

#include <gtest/gtest.h>

#include "lvgl/core/layer.hpp"
#include "lvgl/draw/descriptors.hpp"

namespace {

TEST(Layer, DefaultClipArea) {
  lv::Layer layer;
  // Default-constructed Area is {0,0,0,0} which is a 1x1 pixel — not empty
  EXPECT_EQ(layer.clip_area(), lv::Area{});
  EXPECT_TRUE(layer.tasks().empty());
}

TEST(Layer, SetClipArea) {
  lv::Layer layer;
  lv::Area clip{0, 0, 319, 239};
  layer.set_clip_area(clip);
  EXPECT_EQ(layer.clip_area(), clip);
}

TEST(Layer, ConstructWithClip) {
  lv::Area clip{10, 20, 100, 200};
  lv::Layer layer(clip);
  EXPECT_EQ(layer.clip_area(), clip);
}

TEST(Layer, DrawRectAddsTask) {
  lv::Layer layer(lv::Area{0, 0, 319, 239});

  lv::RectDescriptor dsc;
  dsc.bg_color = lv::Color::Red();
  dsc.radius = 5;

  layer.draw_rect(lv::Area{10, 10, 50, 50}, dsc);

  EXPECT_EQ(layer.tasks().size(), 1u);
  ASSERT_TRUE(std::holds_alternative<lv::RectTask>(layer.tasks()[0]));

  const auto& task = std::get<lv::RectTask>(layer.tasks()[0]);
  EXPECT_EQ(task.bounds, (lv::Area{10, 10, 50, 50}));
  EXPECT_EQ(task.dsc.bg_color, lv::Color::Red());
  EXPECT_EQ(task.dsc.radius, 5);
}

TEST(Layer, DrawLineAddsTask) {
  lv::Layer layer(lv::Area{0, 0, 100, 100});

  lv::LineDescriptor dsc;
  dsc.color = lv::Color::Blue();
  dsc.width = 2;

  layer.draw_line({0, 0}, {100, 100}, dsc);

  EXPECT_EQ(layer.tasks().size(), 1u);
  ASSERT_TRUE(std::holds_alternative<lv::LineTask>(layer.tasks()[0]));
}

TEST(Layer, DrawArcAddsTask) {
  lv::Layer layer(lv::Area{0, 0, 100, 100});

  lv::ArcDescriptor dsc;
  dsc.start_angle = 0;
  dsc.end_angle = 180;

  layer.draw_arc({50, 50}, dsc);

  EXPECT_EQ(layer.tasks().size(), 1u);
  ASSERT_TRUE(std::holds_alternative<lv::ArcTask>(layer.tasks()[0]));
}

TEST(Layer, MultipleTasks) {
  lv::Layer layer(lv::Area{0, 0, 319, 239});

  layer.draw_rect(lv::Area{0, 0, 100, 100}, {});
  layer.draw_rect(lv::Area{50, 50, 150, 150}, {});
  layer.draw_line({0, 0}, {100, 100}, {});

  EXPECT_EQ(layer.tasks().size(), 3u);
}

TEST(Layer, ClearRemovesTasks) {
  lv::Layer layer(lv::Area{0, 0, 319, 239});
  layer.draw_rect(lv::Area{0, 0, 100, 100}, {});
  EXPECT_EQ(layer.tasks().size(), 1u);

  layer.clear();
  EXPECT_TRUE(layer.tasks().empty());
}

TEST(Layer, DrawTriangleAddsTask) {
  lv::Layer layer(lv::Area{0, 0, 100, 100});

  lv::TriangleDescriptor dsc;
  dsc.color = lv::Color::Green();

  layer.draw_triangle({{{0, 0}, {50, 100}, {100, 0}}}, dsc);

  EXPECT_EQ(layer.tasks().size(), 1u);
  ASSERT_TRUE(std::holds_alternative<lv::TriangleTask>(layer.tasks()[0]));
}

TEST(DrawTask, VariantHoldsCorrectTypes) {
  lv::DrawTask task = lv::RectTask{};
  EXPECT_TRUE(std::holds_alternative<lv::RectTask>(task));

  task = lv::LabelTask{};
  EXPECT_TRUE(std::holds_alternative<lv::LabelTask>(task));

  task = lv::ImageTask{};
  EXPECT_TRUE(std::holds_alternative<lv::ImageTask>(task));
}

}  // namespace
