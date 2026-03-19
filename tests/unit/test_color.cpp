// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// tests/unit/test_color.cpp
// Unit tests for include/lvgl/misc/color.hpp
// Coverage targets: Color (construct, from_hex, from_hex32, named colours,
//   with_alpha, mix, comparison), GradStop, ColorFilter,
//   color format tag traits, and color::Active selection.

#include <gtest/gtest.h>

#include "lvgl/misc/color.hpp"

using namespace lv;

// ── Color construction ────────────────────────────────────────────────────────

TEST(Color, DefaultConstruction) {
    Color c;
    EXPECT_EQ(c.r, 0);
    EXPECT_EQ(c.g, 0);
    EXPECT_EQ(c.b, 0);
    EXPECT_EQ(c.a, 255);
}

TEST(Color, ValueConstructionRGB) {
    Color c{128, 64, 32};
    EXPECT_EQ(c.r, 128);
    EXPECT_EQ(c.g,  64);
    EXPECT_EQ(c.b,  32);
    EXPECT_EQ(c.a, 255);  // default alpha
}

TEST(Color, ValueConstructionRGBA) {
    Color c{10, 20, 30, 200};
    EXPECT_EQ(c.r,  10);
    EXPECT_EQ(c.g,  20);
    EXPECT_EQ(c.b,  30);
    EXPECT_EQ(c.a, 200);
}

// ── from_hex / from_hex32 ─────────────────────────────────────────────────────

TEST(Color, FromHexRed) {
    auto c = Color::from_hex(0xFF0000);
    EXPECT_EQ(c.r, 255);
    EXPECT_EQ(c.g,   0);
    EXPECT_EQ(c.b,   0);
    EXPECT_EQ(c.a, 255);
}

TEST(Color, FromHexGreen) {
    auto c = Color::from_hex(0x00FF00);
    EXPECT_EQ(c.r,   0);
    EXPECT_EQ(c.g, 255);
    EXPECT_EQ(c.b,   0);
}

TEST(Color, FromHexBlue) {
    auto c = Color::from_hex(0x0000FF);
    EXPECT_EQ(c.r,   0);
    EXPECT_EQ(c.g,   0);
    EXPECT_EQ(c.b, 255);
}

TEST(Color, FromHexWhite) {
    auto c = Color::from_hex(0xFFFFFF);
    EXPECT_EQ(c.r, 255);
    EXPECT_EQ(c.g, 255);
    EXPECT_EQ(c.b, 255);
}

TEST(Color, FromHexBlack) {
    auto c = Color::from_hex(0x000000);
    EXPECT_EQ(c.r, 0);
    EXPECT_EQ(c.g, 0);
    EXPECT_EQ(c.b, 0);
}

TEST(Color, FromHex32WithAlpha) {
    auto c = Color::from_hex32(0x80FF8040);  // a=0x80, r=0xFF, g=0x80, b=0x40
    EXPECT_EQ(c.r, 0xFF);
    EXPECT_EQ(c.g, 0x80);
    EXPECT_EQ(c.b, 0x40);
    EXPECT_EQ(c.a, 0x80);
}

TEST(Color, FromHex32FullyOpaque) {
    auto c = Color::from_hex32(0xFF123456);
    EXPECT_EQ(c.r, 0x12);
    EXPECT_EQ(c.g, 0x34);
    EXPECT_EQ(c.b, 0x56);
    EXPECT_EQ(c.a, 0xFF);
}

// ── Named colours ─────────────────────────────────────────────────────────────

TEST(Color, Black) {
    auto c = Color::Black();
    EXPECT_EQ(c.r, 0); EXPECT_EQ(c.g, 0); EXPECT_EQ(c.b, 0);
    EXPECT_EQ(c.a, 255);
}

TEST(Color, White) {
    auto c = Color::White();
    EXPECT_EQ(c.r, 255); EXPECT_EQ(c.g, 255); EXPECT_EQ(c.b, 255);
    EXPECT_EQ(c.a, 255);
}

TEST(Color, Red) {
    auto c = Color::Red();
    EXPECT_EQ(c.r, 255); EXPECT_EQ(c.g, 0); EXPECT_EQ(c.b, 0);
}

TEST(Color, Green) {
    auto c = Color::Green();
    EXPECT_EQ(c.r, 0); EXPECT_EQ(c.g, 255); EXPECT_EQ(c.b, 0);
}

TEST(Color, Blue) {
    auto c = Color::Blue();
    EXPECT_EQ(c.r, 0); EXPECT_EQ(c.g, 0); EXPECT_EQ(c.b, 255);
}

TEST(Color, Cyan) {
    auto c = Color::Cyan();
    EXPECT_EQ(c.r, 0); EXPECT_EQ(c.g, 255); EXPECT_EQ(c.b, 255);
}

TEST(Color, Magenta) {
    auto c = Color::Magenta();
    EXPECT_EQ(c.r, 255); EXPECT_EQ(c.g, 0); EXPECT_EQ(c.b, 255);
}

TEST(Color, Yellow) {
    auto c = Color::Yellow();
    EXPECT_EQ(c.r, 255); EXPECT_EQ(c.g, 255); EXPECT_EQ(c.b, 0);
}

TEST(Color, Transp) {
    auto c = Color::Transp();
    EXPECT_EQ(c.a, 0);
}

// ── with_alpha ────────────────────────────────────────────────────────────────

TEST(Color, WithAlphaChangesOnlyAlpha) {
    auto c = Color::Red().with_alpha(128);
    EXPECT_EQ(c.r, 255);
    EXPECT_EQ(c.g,   0);
    EXPECT_EQ(c.b,   0);
    EXPECT_EQ(c.a, 128);
}

TEST(Color, WithAlphaZero) {
    auto c = Color::White().with_alpha(0);
    EXPECT_EQ(c.a, 0);
}

// ── mix ──────────────────────────────────────────────────────────────────────

TEST(Color, MixRatioZeroReturnsSelf) {
    Color a{100, 150, 200, 255};
    Color b{200, 100,  50, 128};
    auto m = a.mix(b, 0);
    EXPECT_EQ(m, a);
}

TEST(Color, MixRatio255ReturnsOther) {
    Color a{0, 0, 0, 255};
    Color b{100, 150, 200, 128};
    auto m = a.mix(b, 255);
    EXPECT_EQ(m, b);
}

TEST(Color, MixRatio128IsApproximatelyMidpoint) {
    Color a{0,   0,   0, 0};
    Color b{200, 100, 50, 200};
    auto m = a.mix(b, 128);
    // Should be roughly half of b's values (within rounding of 1)
    EXPECT_NEAR(m.r, 100, 1);
    EXPECT_NEAR(m.g,  50, 1);
    EXPECT_NEAR(m.b,  25, 1);
    EXPECT_NEAR(m.a, 100, 1);
}

TEST(Color, MixSameColorReturnsItself) {
    Color c{80, 90, 100, 200};
    auto m = c.mix(c, 128);
    EXPECT_EQ(m, c);
}

// ── Comparison ────────────────────────────────────────────────────────────────

TEST(Color, EqualityEqual) {
    EXPECT_EQ(Color(10, 20, 30, 255), Color(10, 20, 30, 255));
}

TEST(Color, EqualityDifferR) {
    EXPECT_NE(Color(10, 20, 30, 255), Color(11, 20, 30, 255));
}

TEST(Color, EqualityDifferG) {
    EXPECT_NE(Color(10, 20, 30, 255), Color(10, 21, 30, 255));
}

TEST(Color, EqualityDifferB) {
    EXPECT_NE(Color(10, 20, 30, 255), Color(10, 20, 31, 255));
}

TEST(Color, EqualityDifferA) {
    EXPECT_NE(Color(10, 20, 30, 255), Color(10, 20, 30, 128));
}

// ── Opacity constants ─────────────────────────────────────────────────────────

TEST(Opacity, OpaTranspIsZero) {
    EXPECT_EQ(OpaTransp, 0);
}

TEST(Opacity, OpaFullIs255) {
    EXPECT_EQ(OpaFull, 255);
}

// ── Color format tag traits ───────────────────────────────────────────────────

TEST(ColorFormat, ARGB8888Traits) {
    EXPECT_EQ(color::ARGB8888::bits_per_pixel, 32);
    EXPECT_TRUE(color::ARGB8888::has_alpha);
}

TEST(ColorFormat, RGB888Traits) {
    EXPECT_EQ(color::RGB888::bits_per_pixel, 24);
    EXPECT_FALSE(color::RGB888::has_alpha);
}

TEST(ColorFormat, RGB565Traits) {
    EXPECT_EQ(color::RGB565::bits_per_pixel, 16);
    EXPECT_FALSE(color::RGB565::has_alpha);
}

TEST(ColorFormat, L8Traits) {
    EXPECT_EQ(color::L8::bits_per_pixel, 8);
    EXPECT_FALSE(color::L8::has_alpha);
}

TEST(ColorFormat, ActiveSatisfiesConcept) {
    // Compile-time concept check
    static_assert(color::ColorFormatTag<color::Active>);
    EXPECT_TRUE(color::Active::bits_per_pixel > 0);
}

// ── GradDir / GradStop / ColorFilter ─────────────────────────────────────────

TEST(GradDir, ValuesDistinct) {
    EXPECT_NE(GradDir::None,    GradDir::Hor);
    EXPECT_NE(GradDir::Hor,     GradDir::Ver);
    EXPECT_NE(GradDir::Linear,  GradDir::Radial);
    EXPECT_NE(GradDir::Radial,  GradDir::Conical);
}

TEST(GradStop, DefaultFracIsZero) {
    GradStop s;
    EXPECT_EQ(s.frac, 0u);
}

TEST(ColorFilter, DefaultDirectionIsNone) {
    ColorFilter cf;
    EXPECT_EQ(cf.dir, GradDir::None);
    EXPECT_EQ(cf.stop_count, 0u);
}
