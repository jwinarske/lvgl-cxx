// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Joel Winarske
//
// Phase 5 unit tests: Widget library

#include <gtest/gtest.h>

#include "lvgl/core/screen.hpp"
#include "lvgl/core/types.hpp"
#include "lvgl/widgets/animimage.hpp"
#include "lvgl/widgets/arc.hpp"
#include "lvgl/widgets/arclabel.hpp"
#include "lvgl/widgets/bar.hpp"
#include "lvgl/widgets/button.hpp"
#include "lvgl/widgets/buttonmatrix.hpp"
#include "lvgl/widgets/calendar.hpp"
#include "lvgl/widgets/canvas.hpp"
#include "lvgl/widgets/chart.hpp"
#include "lvgl/widgets/checkbox.hpp"
#include "lvgl/widgets/container.hpp"
#include "lvgl/widgets/dropdown.hpp"
#include "lvgl/widgets/image.hpp"
#include "lvgl/widgets/imagebutton.hpp"
#include "lvgl/widgets/keyboard.hpp"
#include "lvgl/widgets/label.hpp"
#include "lvgl/widgets/led.hpp"
#include "lvgl/widgets/line.hpp"
#include "lvgl/widgets/list.hpp"
#include "lvgl/widgets/menu.hpp"
#include "lvgl/widgets/msgbox.hpp"
#include "lvgl/widgets/roller.hpp"
#include "lvgl/widgets/scale.hpp"
#include "lvgl/widgets/slider.hpp"
#include "lvgl/widgets/span.hpp"
#include "lvgl/widgets/spinbox.hpp"
#include "lvgl/widgets/spinner.hpp"
#include "lvgl/widgets/switch.hpp"
#include "lvgl/widgets/table.hpp"
#include "lvgl/widgets/tabview.hpp"
#include "lvgl/widgets/textarea.hpp"
#include "lvgl/widgets/tileview.hpp"
#include "lvgl/widgets/window.hpp"

namespace {

// All widgets are tested as children of a Screen (no display needed)
class WidgetTest : public ::testing::Test {
 protected:
  lv::Screen screen;
};

// ── Label ───────────────────────────────────────────────────────────────────

TEST_F(WidgetTest, LabelCreateAndSetText) {
  auto& lbl = screen.create<lv::Label>();
  lbl.set_text(std::string_view{"Hello World"});
  EXPECT_EQ(lbl.text(), "Hello World");
}

TEST_F(WidgetTest, LabelSetTextString) {
  auto& lbl = screen.create<lv::Label>();
  std::string s = "Dynamic";
  lbl.set_text(s);
  EXPECT_EQ(lbl.text(), "Dynamic");
}

TEST_F(WidgetTest, LabelLongMode) {
  auto& lbl = screen.create<lv::Label>();
  lbl.set_long_mode(lv::LabelLongMode::Scroll);
  EXPECT_EQ(lbl.long_mode(), lv::LabelLongMode::Scroll);
}

TEST_F(WidgetTest, LabelRecolor) {
  auto& lbl = screen.create<lv::Label>();
  lbl.set_recolor(true);
  // Just verify it doesn't crash; recolor is an internal flag
}

// ── Button ──────────────────────────────────────────────────────────────────

TEST_F(WidgetTest, ButtonCreate) {
  auto& btn = screen.create<lv::Button>();
  EXPECT_TRUE(btn.has_flag(lv::ObjFlags::Clickable));
}

TEST_F(WidgetTest, ButtonWithLabel) {
  auto& btn = screen.create<lv::Button>();
  auto& lbl = btn.create<lv::Label>();
  lbl.set_text(std::string_view{"OK"});
  EXPECT_EQ(btn.child_count(), 1u);
  EXPECT_EQ(lbl.text(), "OK");
}

// ── Container ───────────────────────────────────────────────────────────────

TEST_F(WidgetTest, ContainerCreate) {
  auto& ctr = screen.create<lv::Container>();
  EXPECT_TRUE(ctr.has_flag(lv::ObjFlags::Scrollable));
}

// ── Slider ──────────────────────────────────────────────────────────────────

TEST_F(WidgetTest, SliderSetValue) {
  auto& s = screen.create<lv::Slider>();
  s.set_value(50);
  EXPECT_EQ(s.value(), 50);
}

TEST_F(WidgetTest, SliderRange) {
  auto& s = screen.create<lv::Slider>();
  s.set_range(-100, 100);
  EXPECT_EQ(s.min_value(), -100);
  EXPECT_EQ(s.max_value(), 100);
}

TEST_F(WidgetTest, SliderMode) {
  auto& s = screen.create<lv::Slider>();
  s.set_mode(lv::SliderMode::Range);
  EXPECT_EQ(s.mode(), lv::SliderMode::Range);
}

TEST_F(WidgetTest, SliderClamps) {
  auto& s = screen.create<lv::Slider>();
  s.set_range(0, 100);
  s.set_value(200);
  EXPECT_EQ(s.value(), 100);
}

// ── Bar ─────────────────────────────────────────────────────────────────────

TEST_F(WidgetTest, BarSetValue) {
  auto& b = screen.create<lv::Bar>();
  b.set_value(75);
  EXPECT_EQ(b.value(), 75);
}

TEST_F(WidgetTest, BarRange) {
  auto& b = screen.create<lv::Bar>();
  b.set_range(0, 200);
  EXPECT_EQ(b.min_value(), 0);
  EXPECT_EQ(b.max_value(), 200);
}

// ── Arc ─────────────────────────────────────────────────────────────────────

TEST_F(WidgetTest, ArcAngles) {
  auto& a = screen.create<lv::Arc>();
  a.set_start_angle(45);
  a.set_end_angle(270);
  EXPECT_EQ(a.angle_start(), 45);
  EXPECT_EQ(a.angle_end(), 270);
}

TEST_F(WidgetTest, ArcValue) {
  auto& a = screen.create<lv::Arc>();
  a.set_range(0, 100);
  a.set_value(60);
  EXPECT_EQ(a.value(), 60);
}

// ── Switch ──────────────────────────────────────────────────────────────────

TEST_F(WidgetTest, SwitchToggle) {
  auto& sw = screen.create<lv::Switch>();
  EXPECT_FALSE(sw.is_checked());
  sw.set_checked(true);
  EXPECT_TRUE(sw.is_checked());
  sw.set_checked(false);
  EXPECT_FALSE(sw.is_checked());
}

TEST_F(WidgetTest, SwitchFlags) {
  auto& sw = screen.create<lv::Switch>();
  EXPECT_TRUE(sw.has_flag(lv::ObjFlags::Clickable));
  EXPECT_TRUE(sw.has_flag(lv::ObjFlags::Checkable));
}

// ── Checkbox ────────────────────────────────────────────────────────────────

TEST_F(WidgetTest, CheckboxText) {
  auto& cb = screen.create<lv::Checkbox>();
  cb.set_text(std::string_view{"Accept terms"});
  EXPECT_EQ(cb.text(), "Accept terms");
}

TEST_F(WidgetTest, CheckboxFlags) {
  auto& cb = screen.create<lv::Checkbox>();
  EXPECT_TRUE(cb.has_flag(lv::ObjFlags::Clickable));
  EXPECT_TRUE(cb.has_flag(lv::ObjFlags::Checkable));
}

// ── Led ─────────────────────────────────────────────────────────────────────

TEST_F(WidgetTest, LedBrightness) {
  auto& led = screen.create<lv::Led>();
  led.set_brightness(200);
  EXPECT_EQ(led.brightness(), 200);
}

TEST_F(WidgetTest, LedOnOffToggle) {
  auto& led = screen.create<lv::Led>();
  led.on();
  EXPECT_EQ(led.brightness(), 255);
  led.off();
  EXPECT_EQ(led.brightness(), 0);
  led.toggle();
  EXPECT_EQ(led.brightness(), 255);
}

TEST_F(WidgetTest, LedColor) {
  auto& led = screen.create<lv::Led>();
  led.set_color(lv::Color::Red());
  EXPECT_EQ(led.color(), lv::Color::Red());
}

// ── Dropdown ────────────────────────────────────────────────────────────────

TEST_F(WidgetTest, DropdownOptions) {
  auto& dd = screen.create<lv::Dropdown>();
  dd.set_options("One\nTwo\nThree");
  EXPECT_EQ(dd.option_count(), 3u);
  EXPECT_EQ(dd.selected(), 0u);
}

TEST_F(WidgetTest, DropdownSelect) {
  auto& dd = screen.create<lv::Dropdown>();
  dd.set_options("A\nB\nC");
  dd.set_selected(2);
  EXPECT_EQ(dd.selected(), 2u);
  EXPECT_EQ(dd.selected_str(), "C");
}

TEST_F(WidgetTest, DropdownOpenClose) {
  auto& dd = screen.create<lv::Dropdown>();
  EXPECT_FALSE(dd.is_open());
  dd.open();
  EXPECT_TRUE(dd.is_open());
  dd.close();
  EXPECT_FALSE(dd.is_open());
}

// ── Roller ──────────────────────────────────────────────────────────────────

TEST_F(WidgetTest, RollerOptions) {
  auto& r = screen.create<lv::Roller>();
  r.set_options("Jan\nFeb\nMar");
  EXPECT_EQ(r.option_count(), 3u);
}

TEST_F(WidgetTest, RollerSelect) {
  auto& r = screen.create<lv::Roller>();
  r.set_options("A\nB\nC");
  r.set_selected(1);
  EXPECT_EQ(r.selected(), 1u);
  EXPECT_EQ(r.selected_str(), "B");
}

// ── TextArea ────────────────────────────────────────────────────────────────

TEST_F(WidgetTest, TextAreaSetText) {
  auto& ta = screen.create<lv::TextArea>();
  ta.set_text("hello");
  EXPECT_EQ(ta.text(), "hello");
}

TEST_F(WidgetTest, TextAreaAddAndDelete) {
  auto& ta = screen.create<lv::TextArea>();
  ta.set_text("");
  ta.add_text("abc");
  EXPECT_EQ(ta.text(), "abc");
  ta.delete_char();
  EXPECT_EQ(ta.text(), "ab");
}

TEST_F(WidgetTest, TextAreaCursorPos) {
  auto& ta = screen.create<lv::TextArea>();
  ta.set_text("hello");
  ta.set_cursor_pos(3);
  EXPECT_EQ(ta.cursor_pos(), 3);
}

TEST_F(WidgetTest, TextAreaPasswordMode) {
  auto& ta = screen.create<lv::TextArea>();
  ta.set_password_mode(true);
  EXPECT_TRUE(ta.is_password());
}

// ── Image ───────────────────────────────────────────────────────────────────

TEST_F(WidgetTest, ImageSetSrc) {
  auto& img = screen.create<lv::Image>();
  int dummy = 42;
  img.set_src(&dummy);
  EXPECT_EQ(img.src(), &dummy);
}

TEST_F(WidgetTest, ImageTransforms) {
  auto& img = screen.create<lv::Image>();
  img.set_offset(10, 20);
  img.set_rotation(900);
  EXPECT_EQ(img.offset_x(), 10);
  EXPECT_EQ(img.offset_y(), 20);
}

// ── Chart ───────────────────────────────────────────────────────────────────

TEST_F(WidgetTest, ChartType) {
  auto& ch = screen.create<lv::Chart>();
  ch.set_type(lv::ChartType::Bar);
  EXPECT_EQ(ch.type(), lv::ChartType::Bar);
}

TEST_F(WidgetTest, ChartPointCount) {
  auto& ch = screen.create<lv::Chart>();
  ch.set_point_count(20);
  EXPECT_EQ(ch.point_count(), 20u);
}

TEST_F(WidgetTest, ChartSeries) {
  auto& ch = screen.create<lv::Chart>();
  ch.set_point_count(5);
  auto& s = ch.add_series(lv::Color::Red(), lv::ChartAxis::Primary);
  ch.set_next_value(s, 42);
  EXPECT_EQ(s.points.size(), 5u);
  // First shifted value should be 42
  EXPECT_EQ(s.points.back(), 42);
}

// ── Table ───────────────────────────────────────────────────────────────────

TEST_F(WidgetTest, TableSetCell) {
  auto& tbl = screen.create<lv::Table>();
  tbl.set_row_count(3);
  tbl.set_column_count(2);
  EXPECT_EQ(tbl.row_count(), 3u);
  EXPECT_EQ(tbl.column_count(), 2u);
  tbl.set_cell_value(1, 0, "Hello");
  EXPECT_EQ(tbl.cell_value(1, 0), "Hello");
}

// ── ButtonMatrix ────────────────────────────────────────────────────────────

TEST_F(WidgetTest, ButtonMatrixCreate) {
  auto& bm = screen.create<lv::ButtonMatrix>();
  std::vector<std::string_view> map = {"A", "B", "C"};
  bm.set_map(map);
  EXPECT_EQ(bm.btn_count(), 3u);
}

// ── Scale ───────────────────────────────────────────────────────────────────

TEST_F(WidgetTest, ScaleRange) {
  auto& sc = screen.create<lv::Scale>();
  sc.set_range(0, 200);
  EXPECT_EQ(sc.min_value(), 0);
  EXPECT_EQ(sc.max_value(), 200);
}

// ── SpinBox ─────────────────────────────────────────────────────────────────

TEST_F(WidgetTest, SpinBoxValue) {
  auto& sb = screen.create<lv::SpinBox>();
  sb.set_value(42);
  EXPECT_EQ(sb.value(), 42);
}

TEST_F(WidgetTest, SpinBoxIncrDecr) {
  auto& sb = screen.create<lv::SpinBox>();
  sb.set_value(10);
  sb.set_step(5);
  sb.increment();
  EXPECT_EQ(sb.value(), 15);
  sb.decrement();
  EXPECT_EQ(sb.value(), 10);
}

// ── Spinner ─────────────────────────────────────────────────────────────────

TEST_F(WidgetTest, SpinnerParams) {
  auto& sp = screen.create<lv::Spinner>();
  sp.set_anim_params(2000, 90);
  EXPECT_EQ(sp.anim_time(), 2000u);
  EXPECT_EQ(sp.arc_length(), 90);
}

// ── ArcLabel ────────────────────────────────────────────────────────────────

TEST_F(WidgetTest, ArcLabelText) {
  auto& al = screen.create<lv::ArcLabel>();
  al.set_text("Hello");
  al.set_radius(50);
  EXPECT_EQ(al.text(), "Hello");
  EXPECT_EQ(al.radius(), 50);
}

// ── Line ────────────────────────────────────────────────────────────────────

TEST_F(WidgetTest, LinePoints) {
  auto& ln = screen.create<lv::Line>();
  std::vector<lv::Point> pts = {{0, 0}, {10, 10}, {20, 0}};
  ln.set_points(pts);
  EXPECT_EQ(ln.point_count(), 3u);
}

// ── Canvas ──────────────────────────────────────────────────────────────────

TEST_F(WidgetTest, CanvasCreate) {
  auto& cv = screen.create<lv::Canvas>();
  EXPECT_EQ(cv.width(), 0);
  EXPECT_EQ(cv.height(), 0);
}

// ── TabView ─────────────────────────────────────────────────────────────────

TEST_F(WidgetTest, TabViewAddTab) {
  auto& tv = screen.create<lv::TabView>();
  tv.add_tab("Tab 1");
  tv.add_tab("Tab 2");
  EXPECT_EQ(tv.tab_count(), 2u);
}

// ── TileView ────────────────────────────────────────────────────────────────

TEST_F(WidgetTest, TileViewAddTile) {
  auto& tv = screen.create<lv::TileView>();
  tv.add_tile(0, 0, lv::Dir::All);
  EXPECT_EQ(tv.child_count(), 1u);
}

// ── Window ──────────────────────────────────────────────────────────────────

TEST_F(WidgetTest, WindowTitle) {
  auto& w = screen.create<lv::Window>();
  w.set_title("My Window");
  EXPECT_EQ(w.title(), "My Window");
}

// ── MsgBox ──────────────────────────────────────────────────────────────────

TEST_F(WidgetTest, MsgBoxTitleAndText) {
  auto& mb = screen.create<lv::MsgBox>();
  mb.set_title("Alert");
  mb.set_text("Something happened");
  EXPECT_EQ(mb.title(), "Alert");
  EXPECT_EQ(mb.text(), "Something happened");
}

// ── Calendar ────────────────────────────────────────────────────────────────

TEST_F(WidgetTest, CalendarDate) {
  auto& cal = screen.create<lv::Calendar>();
  cal.set_today({2026, 3, 19});
  auto today = cal.today();
  EXPECT_EQ(today.year, 2026);
  EXPECT_EQ(today.month, 3);
  EXPECT_EQ(today.day, 19);
}

// ── List ────────────────────────────────────────────────────────────────────

TEST_F(WidgetTest, ListAddItems) {
  auto& lst = screen.create<lv::List>();
  lst.add_text("Header");
  lst.add_btn("", "Item 1");
  EXPECT_EQ(lst.child_count(), 2u);
}

// ── Span ────────────────────────────────────────────────────────────────────

TEST_F(WidgetTest, SpanAddSpans) {
  auto& sp = screen.create<lv::Span>();
  auto& s1 = sp.new_span();
  s1.text = "Hello ";
  auto& s2 = sp.new_span();
  s2.text = "World";
  EXPECT_EQ(sp.span_count(), 2u);
}

// ── Keyboard ────────────────────────────────────────────────────────────────

TEST_F(WidgetTest, KeyboardMode) {
  auto& kb = screen.create<lv::Keyboard>();
  kb.set_mode(lv::KeyboardMode::Number);
  EXPECT_EQ(kb.mode(), lv::KeyboardMode::Number);
}

// ── Menu ────────────────────────────────────────────────────────────────────

TEST_F(WidgetTest, MenuCreatePage) {
  auto& m = screen.create<lv::Menu>();
  auto& page = m.create_page();
  m.set_page(page);
  // Just verify it doesn't crash
}

// ── ImageButton ─────────────────────────────────────────────────────────────

TEST_F(WidgetTest, ImageButtonSrc) {
  auto& ib = screen.create<lv::ImageButton>();
  int dummy = 0;
  ib.set_src(lv::ObjState::Default, &dummy);
  EXPECT_EQ(ib.src(lv::ObjState::Default), &dummy);
}

// ── All widgets can be created as children of Screen ────────────────────────

TEST_F(WidgetTest, AllWidgetsCreateAsChildOfScreen) {
  screen.create<lv::Label>();
  screen.create<lv::Button>();
  screen.create<lv::Container>();
  screen.create<lv::Slider>();
  screen.create<lv::Bar>();
  screen.create<lv::Arc>();
  screen.create<lv::Switch>();
  screen.create<lv::Checkbox>();
  screen.create<lv::Led>();
  screen.create<lv::Dropdown>();
  screen.create<lv::Roller>();
  screen.create<lv::TextArea>();
  screen.create<lv::Image>();
  screen.create<lv::ImageButton>();
  screen.create<lv::Chart>();
  screen.create<lv::Table>();
  screen.create<lv::ButtonMatrix>();
  screen.create<lv::Scale>();
  screen.create<lv::SpinBox>();
  screen.create<lv::Spinner>();
  screen.create<lv::ArcLabel>();
  screen.create<lv::Line>();
  screen.create<lv::Canvas>();
  screen.create<lv::Keyboard>();
  screen.create<lv::TabView>();
  screen.create<lv::TileView>();
  screen.create<lv::Window>();
  screen.create<lv::Menu>();
  screen.create<lv::MsgBox>();
  screen.create<lv::Calendar>();
  screen.create<lv::List>();
  screen.create<lv::Span>();

  EXPECT_EQ(screen.child_count(), 32u);
}

}  // namespace
