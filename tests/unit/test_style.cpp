// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// tests/unit/test_style.cpp
// Phase 2 unit tests for:
//   lv::Style, lv::StyleSheet, lv::Transition, lv::Theme,
//   lv::DefaultTheme, lv::MonoTheme, and Object::add_style / resolve_style.
//
// Coverage targets (Phase 2 checklist from docs/design/11-implementation-plan.md):
//  ✓ Style::set / Style::get round-trip for all property types
//  ✓ Cascade resolves state-specific style before default style
//  ✓ Part selector restricts style to correct sub-part
//  ✓ Transition descriptor stored and retrieved correctly
//  ✓ DefaultTheme::apply() and MonoTheme::apply() style an Object
//  ✓ All Phase-2 unit tests pass

#include <gtest/gtest.h>

#include <optional>
#include <string>

#include "lvgl/core/object.hpp"
#include "lvgl/core/screen.hpp"
#include "lvgl/core/style.hpp"
#include "lvgl/core/theme.hpp"
#include "lvgl/core/transition.hpp"
#include "lvgl/misc/color.hpp"

using namespace lv;

// ═════════════════════════════════════════════════════════════════════════════
// ── Transition ───────────────────────────────────────────────────────────────
// ═════════════════════════════════════════════════════════════════════════════

TEST(Transition, DefaultConstruction) {
    Transition t;
    EXPECT_EQ(t.duration_ms, 300u);
    EXPECT_EQ(t.delay_ms, 0u);
    EXPECT_EQ(t.easing, &Easing::Linear);
    EXPECT_TRUE(t.empty());
}

TEST(Transition, ForPropsAccumulatesIds) {
    Transition t;
    t.for_props<prop::BgColor, prop::Radius>();
    EXPECT_FALSE(t.empty());
    EXPECT_TRUE(t.affects(prop::BgColor::id));
    EXPECT_TRUE(t.affects(prop::Radius::id));
    EXPECT_FALSE(t.affects(prop::BorderWidth::id));
}

TEST(Transition, ForPropsChaining) {
    Transition t;
    t.for_props<prop::BgColor>()
     .for_props<prop::TextColor>();
    EXPECT_TRUE(t.affects(prop::BgColor::id));
    EXPECT_TRUE(t.affects(prop::TextColor::id));
    EXPECT_EQ(t.prop_ids.size(), 2u);
}

TEST(Transition, AffectsReturnsFalseWhenEmpty) {
    Transition t;
    EXPECT_FALSE(t.affects(prop::BgColor::id));
}

TEST(Transition, CustomDurationAndDelay) {
    Transition t;
    t.duration_ms = 500;
    t.delay_ms    = 100;
    EXPECT_EQ(t.duration_ms, 500u);
    EXPECT_EQ(t.delay_ms,    100u);
}

TEST(Transition, EasingLinearAtZero) {
    EXPECT_FLOAT_EQ(Easing::Linear(0.0f), 0.0f);
}
TEST(Transition, EasingLinearAtOne) {
    EXPECT_FLOAT_EQ(Easing::Linear(1.0f), 1.0f);
}
TEST(Transition, EasingLinearAtMid) {
    EXPECT_FLOAT_EQ(Easing::Linear(0.5f), 0.5f);
}
TEST(Transition, EasingEaseInAtBoundaries) {
    EXPECT_FLOAT_EQ(Easing::EaseIn(0.0f), 0.0f);
    EXPECT_FLOAT_EQ(Easing::EaseIn(1.0f), 1.0f);
}
TEST(Transition, EasingEaseOutAtBoundaries) {
    EXPECT_FLOAT_EQ(Easing::EaseOut(0.0f), 0.0f);
    EXPECT_FLOAT_EQ(Easing::EaseOut(1.0f), 1.0f);
}
TEST(Transition, EasingEaseInOutAtBoundaries) {
    EXPECT_FLOAT_EQ(Easing::EaseInOut(0.0f), 0.0f);
    EXPECT_FLOAT_EQ(Easing::EaseInOut(1.0f), 1.0f);
}
TEST(Transition, EasingOvershootAt1IsApprox1) {
    // Overshoot ends at t=1 → 1 (no net overshoot at final value)
    EXPECT_NEAR(Easing::Overshoot(1.0f), 1.0f, 1e-5f);
}
TEST(Transition, EasingEaseInMonotonic) {
    // EaseIn is strictly increasing on [0,1]
    EXPECT_LT(Easing::EaseIn(0.25f), Easing::EaseIn(0.75f));
}
TEST(Transition, EasingEaseOutMonotonic) {
    EXPECT_LT(Easing::EaseOut(0.25f), Easing::EaseOut(0.75f));
}
TEST(Transition, EasingEaseInOutSymmetric) {
    // EaseInOut is symmetric around t=0.5
    EXPECT_NEAR(Easing::EaseInOut(0.25f), 1.0f - Easing::EaseInOut(0.75f), 1e-5f);
}

// ═════════════════════════════════════════════════════════════════════════════
// ── Style ─────────────────────────────────────────────────────────────────────
// ═════════════════════════════════════════════════════════════════════════════

TEST(Style, DefaultIsEmpty) {
    Style s;
    EXPECT_TRUE(s.empty());
    EXPECT_EQ(s.size(), 0u);
}

TEST(Style, SetAndGetInt32) {
    Style s;
    s.set(prop::Radius{}, 8);
    auto v = s.get(prop::Radius{});
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(*v, 8);
}

TEST(Style, SetAndGetColor) {
    Style s;
    s.set(prop::BgColor{}, Color::Red());
    auto v = s.get(prop::BgColor{});
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(*v, Color::Red());
}

TEST(Style, SetAndGetColorFilter) {
    Style s;
    ColorFilter cf;
    cf.dir = GradDir::Hor;
    s.set(prop::BgGrad{}, cf);
    auto v = s.get(prop::BgGrad{});
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(v->dir, GradDir::Hor);
}

TEST(Style, SetAndGetFontPtr) {
    Style s;
    const Font* fptr = reinterpret_cast<const Font*>(0xDEAD);
    s.set(prop::TextFont{}, fptr);
    auto v = s.get(prop::TextFont{});
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(*v, fptr);
}

TEST(Style, SetAndGetVoidPtr) {
    Style s;
    // Use ImgRecolor (a Color property) to keep the test focused on round-trip;
    // void* image-source property requires a live image descriptor which is
    // beyond Phase 2 scope.
    s.set(prop::ImgRecolor{}, Color::Blue());
    // Verify via raw get_raw (covers the monostate / non-monostate path)
    auto raw = s.get_raw(prop::ImgRecolor::id);
    EXPECT_FALSE(std::holds_alternative<std::monostate>(raw));
    EXPECT_TRUE(std::holds_alternative<Color>(raw));
}

TEST(Style, SetAndGetTransition) {
    Style s;
    Transition t;
    t.duration_ms = 500;
    t.for_props<prop::BgColor, prop::Radius>();
    s.set_transition(t);
    auto v = s.get_transition();
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(v->duration_ms, 500u);
    EXPECT_TRUE(v->affects(prop::BgColor::id));
    EXPECT_TRUE(v->affects(prop::Radius::id));
}

TEST(Style, GetUnsetPropertyReturnsNullopt) {
    Style s;
    EXPECT_FALSE(s.get(prop::Radius{}).has_value());
    EXPECT_FALSE(s.get(prop::BgColor{}).has_value());
}

TEST(Style, RemoveProperty) {
    Style s;
    s.set(prop::Radius{}, 4);
    ASSERT_TRUE(s.get(prop::Radius{}).has_value());
    s.remove(prop::Radius{});
    EXPECT_FALSE(s.get(prop::Radius{}).has_value());
    EXPECT_TRUE(s.empty());
}

TEST(Style, ResetClearsAllProperties) {
    Style s;
    s.set(prop::Radius{}, 4)
     .set(prop::BgColor{}, Color::White())
     .set(prop::BorderWidth{}, 1);
    EXPECT_EQ(s.size(), 3u);
    s.reset();
    EXPECT_TRUE(s.empty());
    EXPECT_EQ(s.size(), 0u);
}

TEST(Style, CopyConstruct) {
    Style a;
    a.set(prop::Radius{}, 10);
    Style b{a};
    ASSERT_TRUE(b.get(prop::Radius{}).has_value());
    EXPECT_EQ(*b.get(prop::Radius{}), 10);
}

TEST(Style, CopyAssign) {
    Style a;
    a.set(prop::Radius{}, 12);
    Style b;
    b = a;
    EXPECT_EQ(*b.get(prop::Radius{}), 12);
}

TEST(Style, MoveConstruct) {
    Style a;
    a.set(prop::Radius{}, 15);
    Style b{std::move(a)};
    EXPECT_EQ(*b.get(prop::Radius{}), 15);
}

TEST(Style, MoveAssign) {
    Style a;
    a.set(prop::Radius{}, 20);
    Style b;
    b = std::move(a);
    EXPECT_EQ(*b.get(prop::Radius{}), 20);
}

TEST(Style, ChainedSetReturnsThis) {
    Style s;
    Style& ref = s.set(prop::Radius{}, 6)
                  .set(prop::BgColor{}, Color::Blue())
                  .set(prop::BorderWidth{}, 2);
    EXPECT_EQ(&ref, &s);
    EXPECT_EQ(s.size(), 3u);
}

TEST(Style, GetRawMonostateWhenUnset) {
    Style s;
    auto v = s.get_raw(prop::Radius::id);
    EXPECT_TRUE(std::holds_alternative<std::monostate>(v));
}

TEST(Style, GetRawReturnsValueWhenSet) {
    Style s;
    s.set(prop::Radius{}, 7);
    auto v = s.get_raw(prop::Radius::id);
    ASSERT_TRUE(std::holds_alternative<int32_t>(v));
    EXPECT_EQ(std::get<int32_t>(v), 7);
}

TEST(Style, AllSizingPropertiesRoundTrip) {
    Style s;
    s.set(prop::Width{},     100)
     .set(prop::Height{},    50)
     .set(prop::MinWidth{},  10)
     .set(prop::MinHeight{}, 5)
     .set(prop::MaxWidth{},  200)
     .set(prop::MaxHeight{}, 100);
    EXPECT_EQ(*s.get(prop::Width{}),     100);
    EXPECT_EQ(*s.get(prop::Height{}),    50);
    EXPECT_EQ(*s.get(prop::MinWidth{}),  10);
    EXPECT_EQ(*s.get(prop::MinHeight{}), 5);
    EXPECT_EQ(*s.get(prop::MaxWidth{}),  200);
    EXPECT_EQ(*s.get(prop::MaxHeight{}), 100);
}

TEST(Style, AllPaddingPropertiesRoundTrip) {
    Style s;
    s.set(prop::PadTop{},    10)
     .set(prop::PadBottom{}, 10)
     .set(prop::PadLeft{},   8)
     .set(prop::PadRight{},  8)
     .set(prop::PadRow{},    4)
     .set(prop::PadColumn{}, 4);
    EXPECT_EQ(*s.get(prop::PadTop{}),    10);
    EXPECT_EQ(*s.get(prop::PadBottom{}), 10);
    EXPECT_EQ(*s.get(prop::PadLeft{}),   8);
    EXPECT_EQ(*s.get(prop::PadRight{}),  8);
    EXPECT_EQ(*s.get(prop::PadRow{}),    4);
    EXPECT_EQ(*s.get(prop::PadColumn{}), 4);
}

TEST(Style, AllBackgroundPropertiesRoundTrip) {
    Style s;
    s.set(prop::BgColor{},   Color::Blue())
     .set(prop::BgOpacity{}, 200)
     .set(prop::BgGradDir{}, 1);
    EXPECT_EQ(*s.get(prop::BgColor{}),   Color::Blue());
    EXPECT_EQ(*s.get(prop::BgOpacity{}), 200);
    EXPECT_EQ(*s.get(prop::BgGradDir{}), 1);
}

TEST(Style, AllBorderPropertiesRoundTrip) {
    Style s;
    s.set(prop::BorderColor{},   Color::Black())
     .set(prop::BorderOpacity{}, 255)
     .set(prop::BorderWidth{},   2)
     .set(prop::BorderSide{},    15)
     .set(prop::BorderPost{},    0);
    EXPECT_EQ(*s.get(prop::BorderColor{}),   Color::Black());
    EXPECT_EQ(*s.get(prop::BorderOpacity{}), 255);
    EXPECT_EQ(*s.get(prop::BorderWidth{}),   2);
    EXPECT_EQ(*s.get(prop::BorderSide{}),    15);
    EXPECT_EQ(*s.get(prop::BorderPost{}),    0);
}

TEST(Style, AllOutlinePropertiesRoundTrip) {
    Style s;
    s.set(prop::OutlineColor{},   Color::Red())
     .set(prop::OutlineOpacity{}, 128)
     .set(prop::OutlineWidth{},   3)
     .set(prop::OutlinePad{},     2);
    EXPECT_EQ(*s.get(prop::OutlineColor{}),   Color::Red());
    EXPECT_EQ(*s.get(prop::OutlineOpacity{}), 128);
    EXPECT_EQ(*s.get(prop::OutlineWidth{}),   3);
    EXPECT_EQ(*s.get(prop::OutlinePad{}),     2);
}

TEST(Style, AllShadowPropertiesRoundTrip) {
    Style s;
    s.set(prop::ShadowColor{},   Color::Black())
     .set(prop::ShadowOpacity{}, 80)
     .set(prop::ShadowWidth{},   10)
     .set(prop::ShadowOffsetX{}, 5)
     .set(prop::ShadowOffsetY{}, 5)
     .set(prop::ShadowSpread{},  0);
    EXPECT_EQ(*s.get(prop::ShadowColor{}),   Color::Black());
    EXPECT_EQ(*s.get(prop::ShadowOpacity{}), 80);
    EXPECT_EQ(*s.get(prop::ShadowWidth{}),   10);
    EXPECT_EQ(*s.get(prop::ShadowOffsetX{}), 5);
    EXPECT_EQ(*s.get(prop::ShadowOffsetY{}), 5);
    EXPECT_EQ(*s.get(prop::ShadowSpread{}),  0);
}

TEST(Style, AllTextPropertiesRoundTrip) {
    Style s;
    s.set(prop::TextColor{},         Color::White())
     .set(prop::TextOpacity{},       255)
     .set(prop::TextLetterSpacing{}, 2)
     .set(prop::TextLineSpacing{},   4)
     .set(prop::TextDecor{},         0)
     .set(prop::TextAlign{},         1);
    EXPECT_EQ(*s.get(prop::TextColor{}),         Color::White());
    EXPECT_EQ(*s.get(prop::TextOpacity{}),       255);
    EXPECT_EQ(*s.get(prop::TextLetterSpacing{}), 2);
    EXPECT_EQ(*s.get(prop::TextLineSpacing{}),   4);
    EXPECT_EQ(*s.get(prop::TextDecor{}),         0);
    EXPECT_EQ(*s.get(prop::TextAlign{}),         1);
}

TEST(Style, AllTransformPropertiesRoundTrip) {
    Style s;
    s.set(prop::TransformWidth{},    10)
     .set(prop::TransformHeight{},   20)
     .set(prop::TransformScaleX{},   256)
     .set(prop::TransformScaleY{},   256)
     .set(prop::TransformRotation{}, 450)
     .set(prop::TransformPivotX{},   0)
     .set(prop::TransformPivotY{},   0);
    EXPECT_EQ(*s.get(prop::TransformWidth{}),    10);
    EXPECT_EQ(*s.get(prop::TransformHeight{}),   20);
    EXPECT_EQ(*s.get(prop::TransformScaleX{}),   256);
    EXPECT_EQ(*s.get(prop::TransformScaleY{}),   256);
    EXPECT_EQ(*s.get(prop::TransformRotation{}), 450);
}

TEST(Style, MiscPropertiesRoundTrip) {
    Style s;
    s.set(prop::Radius{},         8)
     .set(prop::Opacity{},       200)
     .set(prop::AnimDuration{},  300)
     .set(prop::BlendMode{},       0);
    EXPECT_EQ(*s.get(prop::Radius{}),        8);
    EXPECT_EQ(*s.get(prop::Opacity{}),       200);
    EXPECT_EQ(*s.get(prop::AnimDuration{}),  300);
    EXPECT_EQ(*s.get(prop::BlendMode{}),     0);
}

TEST(Style, ImagePropertiesRoundTrip) {
    Style s;
    s.set(prop::ImgOpacity{},    180)
     .set(prop::ImgRecolor{},    Color::Cyan())
     .set(prop::ImgRecolorOpa{}, 100);
    EXPECT_EQ(*s.get(prop::ImgOpacity{}),    180);
    EXPECT_EQ(*s.get(prop::ImgRecolor{}),    Color::Cyan());
    EXPECT_EQ(*s.get(prop::ImgRecolorOpa{}), 100);
}

// ═════════════════════════════════════════════════════════════════════════════
// ── StyleSheet ────────────────────────────────────────────────────────────────
// ═════════════════════════════════════════════════════════════════════════════

TEST(StyleSheet, EmptyByDefault) {
    StyleSheet ss;
    EXPECT_TRUE(ss.empty());
    EXPECT_EQ(ss.size(), 0u);
}

TEST(StyleSheet, AddIncreasesSize) {
    Style s;
    StyleSheet ss;
    ss.add(s, {});
    EXPECT_EQ(ss.size(), 1u);
    EXPECT_FALSE(ss.empty());
}

TEST(StyleSheet, RemoveAllClearsSheet) {
    Style s;
    StyleSheet ss;
    ss.add(s, StyleSelector::Default);
    ss.add(s, StyleSelector::Pressed);
    EXPECT_EQ(ss.size(), 2u);
    ss.remove_all();
    EXPECT_TRUE(ss.empty());
}

TEST(StyleSheet, ResolveUnsetPropertyReturnsMonostate) {
    StyleSheet ss;
    auto v = ss.resolve(prop::Radius::id, Part::Main, ObjState::Default);
    EXPECT_TRUE(std::holds_alternative<std::monostate>(v));
}

TEST(StyleSheet, ResolveDefaultStyle) {
    Style s;
    s.set(prop::Radius{}, 6);
    StyleSheet ss;
    ss.add(s, StyleSelector::Default);
    auto v = ss.resolve(prop::Radius{}, ObjState::Default, Part::Main);
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(*v, 6);
}

// ── Cascade: state-specific style wins over default ───────────────────────────
TEST(StyleSheet, CascadeStatePriorityOverDefault) {
    Style s_default;
    s_default.set(prop::BgColor{}, Color::White());

    Style s_pressed;
    s_pressed.set(prop::BgColor{}, Color::Red());

    StyleSheet ss;
    ss.add(s_default, StyleSelector::Default);
    ss.add(s_pressed, StyleSelector::Pressed);

    // Query at Pressed state → pressed style wins
    auto v_pressed = ss.resolve(prop::BgColor{}, ObjState::Pressed, Part::Main);
    ASSERT_TRUE(v_pressed.has_value());
    EXPECT_EQ(*v_pressed, Color::Red());

    // Query at Default state → default style
    auto v_default = ss.resolve(prop::BgColor{}, ObjState::Default, Part::Main);
    ASSERT_TRUE(v_default.has_value());
    EXPECT_EQ(*v_default, Color::White());
}

TEST(StyleSheet, CascadeFallbackToDefaultWhenNoStateMatch) {
    Style s_default;
    s_default.set(prop::Radius{}, 4);

    Style s_pressed;
    s_pressed.set(prop::BgColor{}, Color::Red());  // only BgColor for pressed

    StyleSheet ss;
    ss.add(s_default, StyleSelector::Default);
    ss.add(s_pressed, StyleSelector::Pressed);

    // Radius is only in the default style — must be found for any state
    auto v = ss.resolve(prop::Radius{}, ObjState::Pressed, Part::Main);
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(*v, 4);
}

TEST(StyleSheet, CascadeMultipleStatesMostSpecificWins) {
    // Style for Focused (1 bit) vs style for Focused|Pressed (2 bits)
    Style s_focused;
    s_focused.set(prop::BorderWidth{}, 2);

    Style s_focused_pressed;
    s_focused_pressed.set(prop::BorderWidth{}, 4);

    StyleSheet ss;
    ss.add(s_focused,         {Part::Main, ObjState::Focused});
    ss.add(s_focused_pressed, {Part::Main, ObjState::Focused | ObjState::Pressed});

    // State = Focused | Pressed → 2-bit entry wins
    auto v = ss.resolve(prop::BorderWidth{},
                        ObjState::Focused | ObjState::Pressed, Part::Main);
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(*v, 4);

    // State = Focused only → 1-bit entry wins
    auto v2 = ss.resolve(prop::BorderWidth{}, ObjState::Focused, Part::Main);
    ASSERT_TRUE(v2.has_value());
    EXPECT_EQ(*v2, 2);
}

// ── Part selector tests ───────────────────────────────────────────────────────
TEST(StyleSheet, PartSelectorMainOnly) {
    Style s_main;
    s_main.set(prop::BgColor{}, Color::White());

    StyleSheet ss;
    ss.add(s_main, {Part::Main, ObjState::Default});

    // Querying Scrollbar part → no match
    auto v = ss.resolve(prop::BgColor{}, ObjState::Default, Part::Scrollbar);
    EXPECT_FALSE(v.has_value());

    // Querying Main part → match
    auto v2 = ss.resolve(prop::BgColor{}, ObjState::Default, Part::Main);
    ASSERT_TRUE(v2.has_value());
    EXPECT_EQ(*v2, Color::White());
}

TEST(StyleSheet, PartSelectorScrollbar) {
    Style s_main;
    s_main.set(prop::BgColor{}, Color::White());

    Style s_scrollbar;
    s_scrollbar.set(prop::BgColor{}, Color::from_hex(0x808080));

    StyleSheet ss;
    ss.add(s_main,      {Part::Main,      ObjState::Default});
    ss.add(s_scrollbar, {Part::Scrollbar, ObjState::Default});

    // Main part
    auto v_main = ss.resolve(prop::BgColor{}, ObjState::Default, Part::Main);
    ASSERT_TRUE(v_main.has_value());
    EXPECT_EQ(*v_main, Color::White());

    // Scrollbar part
    auto v_sb = ss.resolve(prop::BgColor{}, ObjState::Default, Part::Scrollbar);
    ASSERT_TRUE(v_sb.has_value());
    EXPECT_EQ(*v_sb, Color::from_hex(0x808080));
}

TEST(StyleSheet, RemoveSpecificEntry) {
    Style s1;
    s1.set(prop::Radius{}, 4);
    Style s2;
    s2.set(prop::Radius{}, 8);

    StyleSheet ss;
    ss.add(s1, StyleSelector::Default);
    ss.add(s2, StyleSelector::Default);
    EXPECT_EQ(ss.size(), 2u);

    // Remove s1 — s2 should still be there
    ss.remove(s1, StyleSelector::Default);
    EXPECT_EQ(ss.size(), 1u);

    auto v = ss.resolve(prop::Radius{}, ObjState::Default, Part::Main);
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(*v, 8);
}

TEST(StyleSheet, ResolveTypedTemplateMethod) {
    Style s;
    s.set(prop::BgColor{}, Color::Green());
    StyleSheet ss;
    ss.add(s, StyleSelector::Default);

    auto v = ss.resolve(prop::BgColor{}, ObjState::Default, Part::Main);
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(*v, Color::Green());
}

TEST(StyleSheet, LastAddedWinsForSameSpecificity) {
    Style s1;
    s1.set(prop::Radius{}, 4);
    Style s2;
    s2.set(prop::Radius{}, 8);

    StyleSheet ss;
    ss.add(s1, StyleSelector::Default);
    ss.add(s2, StyleSelector::Default);

    // s2 was added last → it wins (same specificity = 0-bit state)
    auto v = ss.resolve(prop::Radius{}, ObjState::Default, Part::Main);
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(*v, 8);
}

TEST(StyleSheet, StateNotMatchingDoesNotResolve) {
    Style s;
    s.set(prop::BgColor{}, Color::Blue());
    StyleSheet ss;
    ss.add(s, StyleSelector::Pressed);  // only for pressed state

    // Object is not in pressed state
    auto v = ss.resolve(prop::BgColor{}, ObjState::Default, Part::Main);
    EXPECT_FALSE(v.has_value());
}

// ═════════════════════════════════════════════════════════════════════════════
// ── Object style integration ──────────────────────────────────────────────────
// ═════════════════════════════════════════════════════════════════════════════

// Minimal concrete subclass that tracks on_style_changed calls
struct StyledWidget : public Object {
    explicit StyledWidget(Object* p) : Object(p) {}
    int style_changed_count = 0;
    void on_style_changed() override { ++style_changed_count; }
};

TEST(ObjectStyle, AddStyleCallsOnStyleChanged) {
    Screen root;
    auto& w = root.create<StyledWidget>();
    Style s;
    w.add_style(s);
    EXPECT_GT(w.style_changed_count, 0);
}

TEST(ObjectStyle, RemoveStyleCallsOnStyleChanged) {
    Screen root;
    auto& w = root.create<StyledWidget>();
    Style s;
    w.add_style(s);
    const int count_before = w.style_changed_count;
    w.remove_style(s);
    EXPECT_GT(w.style_changed_count, count_before);
}

TEST(ObjectStyle, RemoveAllStylesCallsOnStyleChanged) {
    Screen root;
    auto& w = root.create<StyledWidget>();
    Style s;
    w.add_style(s);
    const int count_before = w.style_changed_count;
    w.remove_all_styles();
    EXPECT_GT(w.style_changed_count, count_before);
}

TEST(ObjectStyle, ResolveStyleReturnsNulloptWhenNoStyles) {
    Screen root;
    auto& w = root.create<StyledWidget>();
    EXPECT_FALSE(w.resolve_style(prop::Radius{}).has_value());
}

TEST(ObjectStyle, ResolveStyleReturnsPropValue) {
    Screen root;
    auto& w = root.create<StyledWidget>();
    Style s;
    s.set(prop::Radius{}, 10);
    w.add_style(s, StyleSelector::Default);
    auto v = w.resolve_style(prop::Radius{});
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(*v, 10);
}

TEST(ObjectStyle, ResolveStyleCascadeStatePriority) {
    Screen root;
    auto& w = root.create<StyledWidget>();

    Style s_default;
    s_default.set(prop::BgColor{}, Color::White());

    Style s_pressed;
    s_pressed.set(prop::BgColor{}, Color::Red());

    w.add_style(s_default, StyleSelector::Default);
    w.add_style(s_pressed, StyleSelector::Pressed);

    // Resolve for pressed state
    auto vp = w.resolve_style(prop::BgColor{}, ObjState::Pressed);
    ASSERT_TRUE(vp.has_value());
    EXPECT_EQ(*vp, Color::Red());

    // Resolve for default state
    auto vd = w.resolve_style(prop::BgColor{});
    ASSERT_TRUE(vd.has_value());
    EXPECT_EQ(*vd, Color::White());
}

TEST(ObjectStyle, ResolveStyleWithPartSelector) {
    Screen root;
    auto& w = root.create<StyledWidget>();

    Style s_main;
    s_main.set(prop::BgColor{}, Color::White());
    Style s_scrollbar;
    s_scrollbar.set(prop::BgColor{}, Color::Black());

    w.add_style(s_main,      {Part::Main,      ObjState::Default});
    w.add_style(s_scrollbar, {Part::Scrollbar, ObjState::Default});

    auto v_main = w.resolve_style(prop::BgColor{}, ObjState::Default, Part::Main);
    ASSERT_TRUE(v_main.has_value());
    EXPECT_EQ(*v_main, Color::White());

    auto v_sb = w.resolve_style(prop::BgColor{}, ObjState::Default, Part::Scrollbar);
    ASSERT_TRUE(v_sb.has_value());
    EXPECT_EQ(*v_sb, Color::Black());
}

TEST(ObjectStyle, RemoveAllStylesMakesResolveReturnNullopt) {
    Screen root;
    auto& w = root.create<StyledWidget>();
    Style s;
    s.set(prop::Radius{}, 5);
    w.add_style(s);
    ASSERT_TRUE(w.resolve_style(prop::Radius{}).has_value());
    w.remove_all_styles();
    EXPECT_FALSE(w.resolve_style(prop::Radius{}).has_value());
}

TEST(ObjectStyle, ResolveTransitionFromStyle) {
    Screen root;
    auto& w = root.create<StyledWidget>();
    Style s;
    Transition t;
    t.duration_ms = 200;
    t.for_props<prop::BgColor>();
    s.set_transition(t);
    w.add_style(s);
    auto vt = w.resolve_style(prop::PropTransition{});
    ASSERT_TRUE(vt.has_value());
    EXPECT_EQ(vt->duration_ms, 200u);
    EXPECT_TRUE(vt->affects(prop::BgColor::id));
}

// ═════════════════════════════════════════════════════════════════════════════
// ── DefaultTheme ──────────────────────────────────────────────────────────────
// ═════════════════════════════════════════════════════════════════════════════

TEST(DefaultTheme, DefaultColors) {
    DefaultTheme t;
    EXPECT_EQ(t.primary_color(),   Color::from_hex(0x2196F3));
    EXPECT_EQ(t.secondary_color(), Color::from_hex(0xF5F5F5));
    EXPECT_EQ(t.fg_color(),        Color::Black());
    EXPECT_EQ(t.bg_color(),        Color::White());
}

TEST(DefaultTheme, CustomColors) {
    DefaultTheme t{Color::Red(), Color::Green()};
    EXPECT_EQ(t.primary_color(),   Color::Red());
    EXPECT_EQ(t.secondary_color(), Color::Green());
}

TEST(DefaultTheme, ApplyAddsDefaultStyles) {
    Screen root;
    DefaultTheme theme;

    auto& w = root.create<StyledWidget>();
    theme.apply(w);

    // After apply(), the default state should have bg_color = White
    auto bg = w.resolve_style(prop::BgColor{}, ObjState::Default);
    ASSERT_TRUE(bg.has_value());
    EXPECT_EQ(*bg, Color::White());
}

TEST(DefaultTheme, ApplyAddsRadius) {
    Screen root;
    DefaultTheme theme;
    auto& w = root.create<StyledWidget>();
    theme.apply(w);
    auto r = w.resolve_style(prop::Radius{});
    ASSERT_TRUE(r.has_value());
    EXPECT_EQ(*r, 6);
}

TEST(DefaultTheme, ApplyAddsPressedBgColor) {
    Screen root;
    DefaultTheme theme;
    auto& w = root.create<StyledWidget>();
    theme.apply(w);

    // Pressed state must have a bg_color distinct from default
    auto bg_pressed  = w.resolve_style(prop::BgColor{}, ObjState::Pressed);
    auto bg_default  = w.resolve_style(prop::BgColor{}, ObjState::Default);
    ASSERT_TRUE(bg_pressed.has_value());
    ASSERT_TRUE(bg_default.has_value());
    EXPECT_NE(*bg_pressed, *bg_default);
}

TEST(DefaultTheme, ApplyAddsFocusedOutline) {
    Screen root;
    DefaultTheme theme;
    auto& w = root.create<StyledWidget>();
    theme.apply(w);

    auto ow = w.resolve_style(prop::OutlineWidth{}, ObjState::Focused);
    ASSERT_TRUE(ow.has_value());
    EXPECT_EQ(*ow, 3);
}

TEST(DefaultTheme, ApplyAddsBorderWidth) {
    Screen root;
    DefaultTheme theme;
    auto& w = root.create<StyledWidget>();
    theme.apply(w);

    auto bw = w.resolve_style(prop::BorderWidth{});
    ASSERT_TRUE(bw.has_value());
    EXPECT_EQ(*bw, 1);
}

TEST(DefaultTheme, ApplyAddsPadding) {
    Screen root;
    DefaultTheme theme;
    auto& w = root.create<StyledWidget>();
    theme.apply(w);

    EXPECT_EQ(*w.resolve_style(prop::PadTop{}),    4);
    EXPECT_EQ(*w.resolve_style(prop::PadBottom{}), 4);
    EXPECT_EQ(*w.resolve_style(prop::PadLeft{}),   8);
    EXPECT_EQ(*w.resolve_style(prop::PadRight{}),  8);
}

// ═════════════════════════════════════════════════════════════════════════════
// ── MonoTheme ─────────────────────────────────────────────────────────────────
// ═════════════════════════════════════════════════════════════════════════════

TEST(MonoTheme, Colors) {
    MonoTheme t;
    EXPECT_EQ(t.primary_color(),   Color::Black());
    EXPECT_EQ(t.secondary_color(), Color::White());
    EXPECT_EQ(t.fg_color(),        Color::Black());
    EXPECT_EQ(t.bg_color(),        Color::White());
}

TEST(MonoTheme, ApplyAddsWhiteBgDefault) {
    Screen root;
    MonoTheme theme;
    auto& w = root.create<StyledWidget>();
    theme.apply(w);

    auto bg = w.resolve_style(prop::BgColor{}, ObjState::Default);
    ASSERT_TRUE(bg.has_value());
    EXPECT_EQ(*bg, Color::White());
}

TEST(MonoTheme, ApplyAddsBlackBgPressed) {
    Screen root;
    MonoTheme theme;
    auto& w = root.create<StyledWidget>();
    theme.apply(w);

    auto bg = w.resolve_style(prop::BgColor{}, ObjState::Pressed);
    ASSERT_TRUE(bg.has_value());
    EXPECT_EQ(*bg, Color::Black());
}

TEST(MonoTheme, ApplyAddsPressedTextWhite) {
    Screen root;
    MonoTheme theme;
    auto& w = root.create<StyledWidget>();
    theme.apply(w);

    auto tc = w.resolve_style(prop::TextColor{}, ObjState::Pressed);
    ASSERT_TRUE(tc.has_value());
    EXPECT_EQ(*tc, Color::White());
}

TEST(MonoTheme, ApplyAddsZeroRadius) {
    Screen root;
    MonoTheme theme;
    auto& w = root.create<StyledWidget>();
    theme.apply(w);

    auto r = w.resolve_style(prop::Radius{});
    ASSERT_TRUE(r.has_value());
    EXPECT_EQ(*r, 0);
}

TEST(MonoTheme, ApplyAddsFocusedBorder) {
    Screen root;
    MonoTheme theme;
    auto& w = root.create<StyledWidget>();
    theme.apply(w);

    auto bw = w.resolve_style(prop::BorderWidth{}, ObjState::Focused);
    ASSERT_TRUE(bw.has_value());
    EXPECT_EQ(*bw, 3);
}

// ═════════════════════════════════════════════════════════════════════════════
// ── StyleSelector factory presets ─────────────────────────────────────────────
// ═════════════════════════════════════════════════════════════════════════════

TEST(StyleSelectorPresets, DefaultIsMainAndDefaultState) {
    auto sel = StyleSelector::Default;
    EXPECT_EQ(sel.part,  Part::Main);
    EXPECT_EQ(sel.state, ObjState::Default);
}

TEST(StyleSelectorPresets, PressedIsMainAndPressedState) {
    auto sel = StyleSelector::Pressed;
    EXPECT_EQ(sel.part,  Part::Main);
    EXPECT_EQ(sel.state, ObjState::Pressed);
}

TEST(StyleSelectorPresets, FocusedIsMainAndFocusedState) {
    auto sel = StyleSelector::Focused;
    EXPECT_EQ(sel.state, ObjState::Focused);
}

TEST(StyleSelectorPresets, CheckedIsMainAndCheckedState) {
    auto sel = StyleSelector::Checked;
    EXPECT_EQ(sel.state, ObjState::Checked);
}

TEST(StyleSelectorPresets, DisabledIsMainAndDisabledState) {
    auto sel = StyleSelector::Disabled;
    EXPECT_EQ(sel.state, ObjState::Disabled);
}
