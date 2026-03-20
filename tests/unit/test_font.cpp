// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Joel Winarske
//
// Unit tests: Font and text rendering

#include <gtest/gtest.h>

#include "lvgl/core/draw_buffer.hpp"
#include "lvgl/draw/software_renderer.hpp"
#include "lvgl/misc/font.hpp"

namespace {

using Pixel = lv::color::ARGB8888::pixel_type;

// Helper to read pixel without macro comma issue
template <typename P>
const P& px(const lv::DrawBuffer<P>& buf, int32_t r, int32_t c) {
  return buf[r, c];
}

// ── Font basics ─────────────────────────────────────────────────────────────

TEST(Font, BuiltinFontExists) {
  const auto& f = lv::font_builtin_8x16();
  EXPECT_EQ(f.line_height(), 16);
  EXPECT_GT(f.base_line(), 0);
}

TEST(Font, DefaultFontIsMontserrat14) {
  EXPECT_EQ(&lv::font_default(), &lv::font_montserrat_14());
  EXPECT_EQ(lv::font_default().line_height(), 16);
}

TEST(Font, GlyphLookupAscii) {
  const auto& f = lv::font_builtin_8x16();
  lv::GlyphDsc dsc{};

  // 'A' should succeed
  EXPECT_TRUE(f.get_glyph_dsc(dsc, 'A'));
  EXPECT_EQ(dsc.advance_w, 8);
  EXPECT_EQ(dsc.box_w, 8);
  EXPECT_EQ(dsc.box_h, 16);
  EXPECT_NE(dsc.bitmap, nullptr);

  // Space should succeed
  EXPECT_TRUE(f.get_glyph_dsc(dsc, ' '));
  EXPECT_EQ(dsc.advance_w, 8);

  // '~' (last char)
  EXPECT_TRUE(f.get_glyph_dsc(dsc, '~'));
}

TEST(Font, GlyphLookupOutOfRange) {
  const auto& f = lv::font_builtin_8x16();
  lv::GlyphDsc dsc{};

  // Control chars below 32 should fail
  EXPECT_FALSE(f.get_glyph_dsc(dsc, 0));
  EXPECT_FALSE(f.get_glyph_dsc(dsc, 31));

  // DEL and above should fail
  EXPECT_FALSE(f.get_glyph_dsc(dsc, 127));
  EXPECT_FALSE(f.get_glyph_dsc(dsc, 0x100));
}

TEST(Font, GlyphBitmapHasContent) {
  const auto& f = lv::font_builtin_8x16();
  lv::GlyphDsc dsc{};

  // 'A' bitmap should have some non-zero bytes
  EXPECT_TRUE(f.get_glyph_dsc(dsc, 'A'));
  bool has_pixels = false;
  for (int i = 0; i < 16; ++i) {
    if (dsc.bitmap[i] != 0) {
      has_pixels = true;
      break;
    }
  }
  EXPECT_TRUE(has_pixels);

  // Space bitmap should be all zeros
  EXPECT_TRUE(f.get_glyph_dsc(dsc, ' '));
  bool space_empty = true;
  for (int i = 0; i < 16; ++i) {
    if (dsc.bitmap[i] != 0) {
      space_empty = false;
      break;
    }
  }
  EXPECT_TRUE(space_empty);
}

// ── Montserrat fonts ────────────────────────────────────────────────────────

TEST(Font, Montserrat14Metrics) {
  const auto& f = lv::font_montserrat_14();
  EXPECT_EQ(f.line_height(), 16);
  EXPECT_EQ(f.base_line(), 3);
}

TEST(Font, Montserrat14GlyphLookup) {
  const auto& f = lv::font_montserrat_14();
  lv::GlyphDsc dsc{};
  EXPECT_TRUE(f.get_glyph_dsc(dsc, 'A'));
  EXPECT_GT(dsc.advance_w, 0);
  EXPECT_GT(dsc.box_w, 0);
  EXPECT_GT(dsc.box_h, 0);
  EXPECT_EQ(dsc.bpp, 4);
  EXPECT_NE(dsc.bitmap, nullptr);
}

TEST(Font, Montserrat20Metrics) {
  const auto& f = lv::font_montserrat_20();
  EXPECT_EQ(f.line_height(), 22);
  EXPECT_EQ(f.base_line(), 4);
}

TEST(Font, Montserrat24Metrics) {
  const auto& f = lv::font_montserrat_24();
  EXPECT_EQ(f.line_height(), 27);
  EXPECT_EQ(f.base_line(), 5);
}

TEST(Font, Montserrat26Metrics) {
  const auto& f = lv::font_montserrat_26();
  EXPECT_EQ(f.line_height(), 29);
  EXPECT_EQ(f.base_line(), 5);
}

TEST(Font, Montserrat14AntiAliased) {
  // 4 bpp means pixels have 16 alpha levels, not just on/off
  const auto& f = lv::font_montserrat_14();
  lv::GlyphDsc dsc{};
  EXPECT_TRUE(f.get_glyph_dsc(dsc, 'O'));  // 'O' has curved edges → AA
  // Verify some pixels have intermediate alpha values
  bool has_intermediate = false;
  const int32_t ppb = 8 / dsc.bpp;
  const int32_t stride = (dsc.box_w + ppb - 1) / ppb;
  for (int32_t r = 0; r < dsc.box_h && !has_intermediate; ++r) {
    for (int32_t c = 0; c < dsc.box_w; ++c) {
      const int32_t byte_idx = r * stride + c / ppb;
      const int32_t bit_ofs = (ppb - 1 - c % ppb) * dsc.bpp;
      const uint8_t raw = (dsc.bitmap[byte_idx] >> bit_ofs) & 0xF;
      if (raw > 0 && raw < 15) {
        has_intermediate = true;
        break;
      }
    }
  }
  EXPECT_TRUE(has_intermediate) << "'O' should have anti-aliased edge pixels";
}

// ── Text rendering ──────────────────────────────────────────────────────────

TEST(TextRendering, RenderSingleCharToBuffer) {
  auto buf = lv::DrawBuffer<Pixel>::allocate(20, 20);
  // Fill with white
  buf.fill(Pixel{255, 255, 255, 255});

  lv::LabelTask task;
  task.clip_area = {0, 0, 19, 19};
  task.pos = {2, 2};
  task.dsc.text = "A";
  task.dsc.color = lv::Color::Black();
  task.dsc.font = &lv::font_default();
  task.dsc.opa = 255;

  std::vector<lv::DrawTask> tasks;
  tasks.emplace_back(task);

  lv::SoftwareRenderer renderer;
  renderer.execute(buf, tasks);

  // 'A' has pixels set in its bitmap. Check that at least some pixels
  // within the glyph bounding box (2,2)-(9,17) are black.
  int black_count = 0;
  for (int32_t r = 2; r < 18; ++r) {
    for (int32_t c = 2; c < 10; ++c) {
      const auto& p = px(buf, r, c);
      if (p.r == 0 && p.g == 0 && p.b == 0)
        ++black_count;
    }
  }
  EXPECT_GT(black_count, 5);  // 'A' has many set pixels
}

TEST(TextRendering, RenderMultipleChars) {
  auto buf = lv::DrawBuffer<Pixel>::allocate(80, 20);
  buf.fill(Pixel{255, 255, 255, 255});

  lv::LabelTask task;
  task.clip_area = {0, 0, 79, 19};
  task.pos = {0, 0};
  task.dsc.text = "Hello";
  task.dsc.color = lv::Color::Red();
  task.dsc.font = &lv::font_default();
  task.dsc.opa = 255;

  std::vector<lv::DrawTask> tasks;
  tasks.emplace_back(task);

  lv::SoftwareRenderer renderer;
  renderer.execute(buf, tasks);

  // Check that red pixels exist across the width of "Hello" (5 chars × 8px =
  // 40px)
  int red_count = 0;
  for (int32_t r = 0; r < 16; ++r) {
    for (int32_t c = 0; c < 40; ++c) {
      const auto& p = px(buf, r, c);
      if (p.r == 255 && p.g == 0 && p.b == 0)
        ++red_count;
    }
  }
  EXPECT_GT(red_count, 20);  // "Hello" should have many red pixels
}

TEST(TextRendering, ClipAreaRespectsed) {
  auto buf = lv::DrawBuffer<Pixel>::allocate(40, 20);
  buf.fill(Pixel{255, 255, 255, 255});

  lv::LabelTask task;
  // Only allow rendering in left half
  task.clip_area = {0, 0, 19, 19};
  task.pos = {0, 0};
  task.dsc.text = "XXXXX";
  task.dsc.color = lv::Color::Black();
  task.dsc.font = &lv::font_default();
  task.dsc.opa = 255;

  std::vector<lv::DrawTask> tasks;
  tasks.emplace_back(task);

  lv::SoftwareRenderer renderer;
  renderer.execute(buf, tasks);

  // Right half (col 20-39) should be untouched (white)
  for (int32_t r = 0; r < 16; ++r) {
    for (int32_t c = 20; c < 40; ++c) {
      const auto& p = px(buf, r, c);
      EXPECT_EQ(p.r, 255) << "at row=" << r << " col=" << c;
    }
  }
}

TEST(TextRendering, NewlineAdvancesRow) {
  auto buf = lv::DrawBuffer<Pixel>::allocate(20, 40);
  buf.fill(Pixel{255, 255, 255, 255});

  lv::LabelTask task;
  task.clip_area = {0, 0, 19, 39};
  task.pos = {0, 0};
  task.dsc.text = "A\nB";
  task.dsc.color = lv::Color::Black();
  task.dsc.font = &lv::font_default();
  task.dsc.opa = 255;

  std::vector<lv::DrawTask> tasks;
  tasks.emplace_back(task);

  lv::SoftwareRenderer renderer;
  renderer.execute(buf, tasks);

  // 'A' is at rows 0-15, 'B' is at rows 16-31
  // Check that row 20 (inside 'B') has some black pixels
  int black_in_b = 0;
  for (int32_t c = 0; c < 10; ++c) {
    const auto& p = px(buf, 20, c);
    if (p.r == 0 && p.g == 0 && p.b == 0)
      ++black_in_b;
  }
  EXPECT_GT(black_in_b, 0);
}

}  // namespace
