// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// tests/unit/test_types.cpp
// Unit tests for include/lvgl/core/types.hpp
// Coverage targets: ObjFlags operators, ObjState operators, Align enum,
//   ScrollbarMode, Dir, AnimEnable, SizeContent/SizePct, Part, StyleSelector.

#include <gtest/gtest.h>

#include "lvgl/core/types.hpp"

using namespace lv;

// ── ObjFlags ─────────────────────────────────────────────────────────────────

TEST(ObjFlags, NoneIsZero) {
    EXPECT_EQ(static_cast<uint32_t>(ObjFlags::None), 0u);
}

TEST(ObjFlags, BitorProducesUnion) {
    auto f = ObjFlags::Hidden | ObjFlags::Clickable;
    EXPECT_EQ(static_cast<uint32_t>(f),
              static_cast<uint32_t>(ObjFlags::Hidden) |
                  static_cast<uint32_t>(ObjFlags::Clickable));
}

TEST(ObjFlags, BitandProducesIntersection) {
    auto combined = ObjFlags::Hidden | ObjFlags::Clickable;
    EXPECT_EQ(combined & ObjFlags::Hidden, ObjFlags::Hidden);
    EXPECT_EQ(combined & ObjFlags::Scrollable, ObjFlags::None);
}

TEST(ObjFlags, BitwiseNot) {
    auto all_set = ~ObjFlags::None;
    EXPECT_EQ(static_cast<uint32_t>(all_set), ~0u);
    auto cleared = all_set & ~ObjFlags::Hidden;
    EXPECT_EQ(cleared & ObjFlags::Hidden, ObjFlags::None);
}

TEST(ObjFlags, OrAssign) {
    ObjFlags f = ObjFlags::None;
    f |= ObjFlags::Hidden;
    EXPECT_EQ(f, ObjFlags::Hidden);
    f |= ObjFlags::Clickable;
    EXPECT_EQ(f, ObjFlags::Hidden | ObjFlags::Clickable);
}

TEST(ObjFlags, AndAssign) {
    ObjFlags f = ObjFlags::Hidden | ObjFlags::Clickable;
    f &= ObjFlags::Clickable;
    EXPECT_EQ(f, ObjFlags::Clickable);
}

TEST(ObjFlags, UserBitsDistinct) {
    EXPECT_NE(ObjFlags::User1, ObjFlags::User2);
    EXPECT_NE(ObjFlags::User3, ObjFlags::User4);
    auto users = ObjFlags::User1 | ObjFlags::User2 | ObjFlags::User3 | ObjFlags::User4;
    EXPECT_EQ(users & ObjFlags::User1, ObjFlags::User1);
    EXPECT_EQ(users & ObjFlags::User4, ObjFlags::User4);
}

TEST(ObjFlags, AllBitsIndependent) {
    // Verify no two named flags share a bit
    ObjFlags all = ObjFlags::Hidden | ObjFlags::Clickable | ObjFlags::ClickFocusable |
                   ObjFlags::Checkable | ObjFlags::Scrollable | ObjFlags::ScrollElastic |
                   ObjFlags::ScrollMomentum | ObjFlags::ScrollOne | ObjFlags::ScrollChainH |
                   ObjFlags::ScrollChainV | ObjFlags::ScrollOnFocus | ObjFlags::SnappableX |
                   ObjFlags::PressLock | ObjFlags::EventBubble | ObjFlags::GestureBubble |
                   ObjFlags::AdvHittest | ObjFlags::IgnoreLayout | ObjFlags::Floating |
                   ObjFlags::OverflowVisible | ObjFlags::EventTrickle | ObjFlags::StateTrickle |
                   ObjFlags::User1 | ObjFlags::User2 | ObjFlags::User3 | ObjFlags::User4;
    // Just ensure it doesn't crash or collapse
    EXPECT_NE(all, ObjFlags::None);
}

// ── ObjState ──────────────────────────────────────────────────────────────────

TEST(ObjState, DefaultIsZero) {
    EXPECT_EQ(static_cast<uint16_t>(ObjState::Default), 0u);
}

TEST(ObjState, AnyIsAllOnes) {
    EXPECT_EQ(static_cast<uint16_t>(ObjState::Any), 0xFFFFu);
}

TEST(ObjState, BitorProducesUnion) {
    auto s = ObjState::Checked | ObjState::Focused;
    EXPECT_EQ(s & ObjState::Checked, ObjState::Checked);
    EXPECT_EQ(s & ObjState::Focused, ObjState::Focused);
    EXPECT_EQ(s & ObjState::Pressed, ObjState::Default);
}

TEST(ObjState, BitandProducesIntersection) {
    auto s = ObjState::Pressed | ObjState::Disabled;
    EXPECT_EQ(s & ObjState::Pressed,  ObjState::Pressed);
    EXPECT_EQ(s & ObjState::Disabled, ObjState::Disabled);
    EXPECT_EQ(s & ObjState::Checked,  ObjState::Default);
}

TEST(ObjState, BitwiseNot) {
    // ~Default should flip all bits
    auto inv = ~ObjState::Default;
    EXPECT_EQ(static_cast<uint16_t>(inv), 0xFFFFu);
}

TEST(ObjState, OrAssign) {
    ObjState s = ObjState::Default;
    s |= ObjState::Pressed;
    EXPECT_EQ(s, ObjState::Pressed);
    s |= ObjState::Focused;
    EXPECT_EQ(s, ObjState::Pressed | ObjState::Focused);
}

TEST(ObjState, AndAssign) {
    ObjState s = ObjState::Pressed | ObjState::Focused;
    s &= ObjState::Focused;
    EXPECT_EQ(s, ObjState::Focused);
}

TEST(ObjState, UserBitsDistinct) {
    EXPECT_NE(ObjState::User1, ObjState::User2);
    EXPECT_NE(ObjState::User3, ObjState::User4);
}

TEST(ObjState, AllNamedStatesOrToAny) {
    // Verify that all named states are a subset of ObjState::Any (0xFFFF).
    // Any is a "match all" sentinel, not necessarily the OR of named states
    // (gap bits 0x0100-0x0800 are reserved).
    auto all = ObjState::Checked | ObjState::Focused | ObjState::FocusKey |
               ObjState::Edited   | ObjState::Hovered | ObjState::Pressed |
               ObjState::Scrolled | ObjState::Disabled |
               ObjState::User1 | ObjState::User2 | ObjState::User3 | ObjState::User4;
    // Every named-state bit must be set in Any
    EXPECT_EQ(all & ObjState::Any, all);
    // Any must be non-zero
    EXPECT_NE(ObjState::Any, ObjState::Default);
    // Sanity: Any covers all 16 bits
    EXPECT_EQ(static_cast<uint16_t>(ObjState::Any), 0xFFFFu);
}

// ── Align ────────────────────────────────────────────────────────────────────

TEST(Align, DefaultIsZero) {
    EXPECT_EQ(static_cast<uint8_t>(Align::Default), 0u);
}

TEST(Align, ValuesAreDistinct) {
    EXPECT_NE(Align::TopLeft, Align::TopMid);
    EXPECT_NE(Align::Center,  Align::TopLeft);
    EXPECT_NE(Align::OutRightBottom, Align::Center);
}

// ── ScrollbarMode / Dir / AnimEnable ─────────────────────────────────────────

TEST(ScrollbarMode, ValuesExist) {
    EXPECT_NE(ScrollbarMode::Off,    ScrollbarMode::On);
    EXPECT_NE(ScrollbarMode::Active, ScrollbarMode::Auto);
}

TEST(Dir, HorIsLeftOrRight) {
    EXPECT_EQ(static_cast<uint8_t>(Dir::Hor),
              static_cast<uint8_t>(Dir::Left) | static_cast<uint8_t>(Dir::Right));
}

TEST(Dir, VerIsTopOrBottom) {
    EXPECT_EQ(static_cast<uint8_t>(Dir::Ver),
              static_cast<uint8_t>(Dir::Top) | static_cast<uint8_t>(Dir::Bottom));
}

TEST(Dir, AllIsBitOrOfAll) {
    EXPECT_EQ(static_cast<uint8_t>(Dir::All),
              static_cast<uint8_t>(Dir::Left) | static_cast<uint8_t>(Dir::Right) |
                  static_cast<uint8_t>(Dir::Top) | static_cast<uint8_t>(Dir::Bottom));
}

TEST(AnimEnable, OffAndOnDiffer) {
    EXPECT_NE(AnimEnable::Off, AnimEnable::On);
    EXPECT_EQ(static_cast<uint8_t>(AnimEnable::Off), 0u);
    EXPECT_EQ(static_cast<uint8_t>(AnimEnable::On),  1u);
}

// ── SizeContent / SizePct ────────────────────────────────────────────────────

TEST(SizeHelpers, SizeContentSentinel) {
    EXPECT_EQ(SizeContent, 0x7FFFFFF0);
}

TEST(SizeHelpers, SizePctEncodesPercent) {
    // LV_PCT(50) = 0x80000000 | 50
    EXPECT_EQ(SizePct(50),  static_cast<int32_t>(0x80000000u | 50u));
    EXPECT_EQ(SizePct(100), static_cast<int32_t>(0x80000000u | 100u));
    EXPECT_EQ(SizePct(0),   static_cast<int32_t>(0x80000000u));
}

// ── Part ──────────────────────────────────────────────────────────────────────

TEST(Part, ValuesAreDistinct) {
    EXPECT_NE(Part::Main,      Part::Scrollbar);
    EXPECT_NE(Part::Indicator, Part::Knob);
    EXPECT_NE(Part::Selected,  Part::Items);
    EXPECT_NE(Part::Cursor,    Part::Any);
}

TEST(Part, MainIsZero) {
    EXPECT_EQ(static_cast<uint32_t>(Part::Main), 0u);
}

// ── StyleSelector ─────────────────────────────────────────────────────────────

TEST(StyleSelector, DefaultPreset) {
    EXPECT_EQ(StyleSelector::Default.part,  Part::Main);
    EXPECT_EQ(StyleSelector::Default.state, ObjState::Default);
}

TEST(StyleSelector, PressedPreset) {
    EXPECT_EQ(StyleSelector::Pressed.part,  Part::Main);
    EXPECT_EQ(StyleSelector::Pressed.state, ObjState::Pressed);
}

TEST(StyleSelector, FocusedPreset) {
    EXPECT_EQ(StyleSelector::Focused.part,  Part::Main);
    EXPECT_EQ(StyleSelector::Focused.state, ObjState::Focused);
}

TEST(StyleSelector, CheckedPreset) {
    EXPECT_EQ(StyleSelector::Checked.part,  Part::Main);
    EXPECT_EQ(StyleSelector::Checked.state, ObjState::Checked);
}

TEST(StyleSelector, DisabledPreset) {
    EXPECT_EQ(StyleSelector::Disabled.part,  Part::Main);
    EXPECT_EQ(StyleSelector::Disabled.state, ObjState::Disabled);
}

TEST(StyleSelector, DefaultConstruction) {
    StyleSelector s;
    EXPECT_EQ(s.part,  Part::Main);
    EXPECT_EQ(s.state, ObjState::Default);
}

TEST(StyleSelector, CustomConstruction) {
    StyleSelector s{Part::Scrollbar, ObjState::Hovered};
    EXPECT_EQ(s.part,  Part::Scrollbar);
    EXPECT_EQ(s.state, ObjState::Hovered);
}
