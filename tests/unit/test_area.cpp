// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// tests/unit/test_area.cpp
// Unit tests for include/lvgl/misc/area.hpp
// Coverage targets: Point (construct, operators, comparison),
//   Area (construct, from_size, width/height, contains, intersects,
//         intersect, unite, empty, comparison).

#include <gtest/gtest.h>

#include "lvgl/misc/area.hpp"

using namespace lv;

// ── Point
// ─────────────────────────────────────────────────────────────────────

TEST(Point, DefaultConstruction) {
  Point p;
  EXPECT_EQ(p.x, 0);
  EXPECT_EQ(p.y, 0);
}

TEST(Point, ValueConstruction) {
  Point p{3, -7};
  EXPECT_EQ(p.x, 3);
  EXPECT_EQ(p.y, -7);
}

TEST(Point, Addition) {
  Point a{1, 2};
  Point b{3, 4};
  auto c = a + b;
  EXPECT_EQ(c.x, 4);
  EXPECT_EQ(c.y, 6);
}

TEST(Point, Subtraction) {
  Point a{10, 5};
  Point b{3, 8};
  auto c = a - b;
  EXPECT_EQ(c.x, 7);
  EXPECT_EQ(c.y, -3);
}

TEST(Point, AdditionNegativeCoords) {
  Point a{-5, -3};
  Point b{-2, -4};
  auto c = a + b;
  EXPECT_EQ(c.x, -7);
  EXPECT_EQ(c.y, -7);
}

TEST(Point, EqualityEqual) {
  EXPECT_EQ(Point(1, 2), Point(1, 2));
}

TEST(Point, EqualityNotEqual) {
  EXPECT_NE(Point(1, 2), Point(1, 3));
  EXPECT_NE(Point(1, 2), Point(2, 2));
}

TEST(Point, SpaceshipOrdering) {
  // operator<=> should provide all comparison operators
  Point a{0, 0};
  Point b{0, 1};
  EXPECT_TRUE(a < b || b < a || a == b || a > b);  // spaceship is available
}

// ── Area
// ──────────────────────────────────────────────────────────────────────

TEST(Area, DefaultConstruction) {
  Area a;
  EXPECT_EQ(a.x1, 0);
  EXPECT_EQ(a.y1, 0);
  EXPECT_EQ(a.x2, 0);
  EXPECT_EQ(a.y2, 0);
}

TEST(Area, ValueConstruction) {
  Area a{1, 2, 10, 20};
  EXPECT_EQ(a.x1, 1);
  EXPECT_EQ(a.y1, 2);
  EXPECT_EQ(a.x2, 10);
  EXPECT_EQ(a.y2, 20);
}

TEST(Area, FromSize) {
  auto a = Area::from_size(5, 10, 40, 30);
  EXPECT_EQ(a.x1, 5);
  EXPECT_EQ(a.y1, 10);
  EXPECT_EQ(a.x2, 44);  // x + w - 1 = 5 + 40 - 1
  EXPECT_EQ(a.y2, 39);  // y + h - 1 = 10 + 30 - 1
}

TEST(Area, FromSizeZeroOrigin) {
  auto a = Area::from_size(0, 0, 100, 50);
  EXPECT_EQ(a.width(), 100);
  EXPECT_EQ(a.height(), 50);
}

TEST(Area, WidthAndHeight) {
  Area a{0, 0, 99, 49};
  EXPECT_EQ(a.width(), 100);
  EXPECT_EQ(a.height(), 50);
}

TEST(Area, WidthHeightSinglePixel) {
  Area a{5, 5, 5, 5};
  EXPECT_EQ(a.width(), 1);
  EXPECT_EQ(a.height(), 1);
}

TEST(Area, ContainsInsidePoint) {
  Area a{0, 0, 99, 99};
  EXPECT_TRUE(a.contains({50, 50}));
}

TEST(Area, ContainsCornerPoints) {
  Area a{0, 0, 99, 99};
  EXPECT_TRUE(a.contains({0, 0}));
  EXPECT_TRUE(a.contains({99, 0}));
  EXPECT_TRUE(a.contains({0, 99}));
  EXPECT_TRUE(a.contains({99, 99}));
}

TEST(Area, ContainsOutsidePoint) {
  Area a{0, 0, 99, 99};
  EXPECT_FALSE(a.contains({100, 50}));
  EXPECT_FALSE(a.contains({50, 100}));
  EXPECT_FALSE(a.contains({-1, 50}));
  EXPECT_FALSE(a.contains({50, -1}));
}

TEST(Area, IntersectsOverlapping) {
  Area a{0, 0, 10, 10};
  Area b{5, 5, 15, 15};
  EXPECT_TRUE(a.intersects(b));
  EXPECT_TRUE(b.intersects(a));
}

TEST(Area, IntersectsTouching) {
  Area a{0, 0, 10, 10};
  Area b{10, 0, 20, 10};
  EXPECT_TRUE(a.intersects(b));  // touching edge counts
}

TEST(Area, IntersectsNonOverlapping) {
  Area a{0, 0, 9, 9};
  Area b{10, 10, 20, 20};
  EXPECT_FALSE(a.intersects(b));
}

TEST(Area, IntersectsFullyContained) {
  Area outer{0, 0, 100, 100};
  Area inner{10, 10, 50, 50};
  EXPECT_TRUE(outer.intersects(inner));
  EXPECT_TRUE(inner.intersects(outer));
}

TEST(Area, IntersectResult) {
  Area a{0, 0, 10, 10};
  Area b{5, 5, 15, 15};
  auto r = a.intersect(b);
  EXPECT_EQ(r.x1, 5);
  EXPECT_EQ(r.y1, 5);
  EXPECT_EQ(r.x2, 10);
  EXPECT_EQ(r.y2, 10);
}

TEST(Area, IntersectNoOverlapYieldsEmpty) {
  Area a{0, 0, 5, 5};
  Area b{10, 10, 20, 20};
  auto r = a.intersect(b);
  EXPECT_TRUE(r.empty());
}

TEST(Area, UniteResultCoverssBoth) {
  Area a{0, 0, 5, 5};
  Area b{10, 10, 20, 20};
  auto u = a.unite(b);
  EXPECT_EQ(u.x1, 0);
  EXPECT_EQ(u.y1, 0);
  EXPECT_EQ(u.x2, 20);
  EXPECT_EQ(u.y2, 20);
}

TEST(Area, UniteSameArea) {
  Area a{3, 4, 10, 12};
  auto u = a.unite(a);
  EXPECT_EQ(u, a);
}

TEST(Area, EmptyFalseForValidArea) {
  EXPECT_FALSE(Area(0, 0, 1, 1).empty());
  EXPECT_FALSE(Area(0, 0, 0, 0).empty());  // single pixel is NOT empty
}

TEST(Area, EmptyTrueWhenInverted) {
  EXPECT_TRUE(Area(5, 5, 4, 5).empty());  // x2 < x1
  EXPECT_TRUE(Area(5, 5, 5, 4).empty());  // y2 < y1
}

TEST(Area, EqualityEqual) {
  EXPECT_EQ(Area(1, 2, 3, 4), Area(1, 2, 3, 4));
}

TEST(Area, EqualityNotEqual) {
  EXPECT_NE(Area(1, 2, 3, 4), Area(1, 2, 3, 5));
}
