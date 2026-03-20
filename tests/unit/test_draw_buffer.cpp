// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Joel Winarske
//
// Phase 4 unit tests: DrawBuffer

#include <gtest/gtest.h>

#include "lvgl/core/draw_buffer.hpp"
#include "lvgl/misc/color.hpp"

namespace {

using Pixel = lv::color::ARGB8888::pixel_type;

// Helper to access DrawBuffer pixels without tripping the preprocessor comma.
template <typename P>
P& px(lv::DrawBuffer<P>& buf, int32_t r, int32_t c) {
  return buf[r, c];
}
template <typename P>
const P& px(const lv::DrawBuffer<P>& buf, int32_t r, int32_t c) {
  return buf[r, c];
}

TEST(DrawBuffer, NonOwningConstruction) {
  Pixel storage[10 * 20]{};
  lv::DrawBuffer<Pixel> buf(storage, 20, 10);

  EXPECT_EQ(buf.width(), 20);
  EXPECT_EQ(buf.height(), 10);
  EXPECT_EQ(buf.data(), storage);
}

TEST(DrawBuffer, AllocateOwning) {
  auto buf = lv::DrawBuffer<Pixel>::allocate(100, 50);

  EXPECT_EQ(buf.width(), 100);
  EXPECT_EQ(buf.height(), 50);
  EXPECT_NE(buf.data(), nullptr);
}

TEST(DrawBuffer, MultidimensionalSubscript) {
  auto buf = lv::DrawBuffer<Pixel>::allocate(10, 10);

  Pixel val{0, 128, 255, 255};
  px(buf, 3, 5) = val;

  EXPECT_EQ(px(buf, 3, 5).r, 255);
  EXPECT_EQ(px(buf, 3, 5).g, 128);
  EXPECT_EQ(px(buf, 3, 5).b, 0);
  EXPECT_EQ(px(buf, 3, 5).a, 255);
}

TEST(DrawBuffer, FillSetsAllPixels) {
  auto buf = lv::DrawBuffer<Pixel>::allocate(4, 3);
  Pixel fill_val{10, 20, 30, 40};
  buf.fill(fill_val);

  for (int32_t r = 0; r < 3; ++r) {
    for (int32_t c = 0; c < 4; ++c) {
      EXPECT_EQ(px(buf, r, c).r, 30);
      EXPECT_EQ(px(buf, r, c).g, 20);
      EXPECT_EQ(px(buf, r, c).b, 10);
    }
  }
}

TEST(DrawBuffer, ConstSubscriptAccess) {
  auto buf = lv::DrawBuffer<Pixel>::allocate(5, 5);
  px(buf, 2, 3) = Pixel{1, 2, 3, 4};

  const auto& const_buf = buf;
  EXPECT_EQ(px(const_buf, 2, 3).r, 3);
  EXPECT_EQ(px(const_buf, 2, 3).g, 2);
}

TEST(DrawBuffer, RowMajorLayout) {
  Pixel storage[3 * 4]{};
  lv::DrawBuffer<Pixel> buf(storage, 4, 3);
  px(buf, 1, 2) = Pixel{99, 0, 0, 0};

  // Row 1, Col 2 => index 1*4 + 2 = 6
  EXPECT_EQ(storage[6].b, 99);
}

}  // namespace
