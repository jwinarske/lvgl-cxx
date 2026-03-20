// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Joel Winarske
//
// Phase 6 unit tests: Drivers, input devices, integration

#include <gtest/gtest.h>

#include "lvgl/core/display.hpp"
#include "lvgl/core/event.hpp"
#include "lvgl/core/screen.hpp"
#include "lvgl/core/style.hpp"
#include "lvgl/drivers/concepts.hpp"
#include "lvgl/drivers/input_device.hpp"
#include "lvgl/tick/tick.hpp"
#include "lvgl/widgets/button.hpp"
#include "lvgl/widgets/label.hpp"

namespace {

using Pixel = lv::Display::Pixel;

// ── InputState ──────────────────────────────────────────────────────────────

TEST(InputState, DefaultValues) {
  lv::drivers::InputState state;
  EXPECT_EQ(state.type, lv::drivers::InputState::Type::None);
  EXPECT_FALSE(state.pressed);
  EXPECT_EQ(state.x, 0);
  EXPECT_EQ(state.y, 0);
}

// ── Pointer ─────────────────────────────────────────────────────────────────

TEST(Pointer, DefaultState) {
  lv::Pointer ptr;
  EXPECT_EQ(ptr.x(), 0);
  EXPECT_EQ(ptr.y(), 0);
  EXPECT_FALSE(ptr.pressed());
}

TEST(Pointer, SetReadAndPoll) {
  lv::Pointer ptr;
  ptr.set_read([](lv::drivers::InputState& s) {
    s.pressed = true;
    s.x = 100;
    s.y = 50;
  });

  ptr.poll();

  EXPECT_TRUE(ptr.pressed());
  EXPECT_EQ(ptr.x(), 100);
  EXPECT_EQ(ptr.y(), 50);
}

TEST(Pointer, SetDisplay) {
  std::vector<Pixel> buf(10 * 10);
  lv::Display disp(
      10, 10, [](lv::Display&, const lv::Area&, lv::BufferView2D<Pixel>) {});

  lv::Pointer ptr;
  ptr.set_display(disp);
  EXPECT_EQ(ptr.display(), &disp);
}

// ── Keypad ──────────────────────────────────────────────────────────────────

TEST(Keypad, DefaultState) {
  lv::Keypad kp;
  EXPECT_EQ(kp.key(), 0u);
  EXPECT_FALSE(kp.pressed());
}

TEST(Keypad, ReadCallback) {
  lv::Keypad kp;
  kp.set_read([](lv::drivers::InputState& s) {
    s.key = 13;  // Enter
    s.pressed = true;
  });

  kp.poll();
  EXPECT_EQ(kp.key(), 13u);
  EXPECT_TRUE(kp.pressed());
}

// ── Encoder ─────────────────────────────────────────────────────────────────

TEST(Encoder, ReadCallback) {
  lv::Encoder enc;
  enc.set_read([](lv::drivers::InputState& s) {
    s.encoder_diff = 3;
    s.pressed = false;
  });

  enc.poll();
  EXPECT_EQ(enc.diff(), 3);
  EXPECT_FALSE(enc.pressed());
}

// ── DisplayDriver concept ───────────────────────────────────────────────────

struct MockDisplayDriver {
  void init() {}
  void deinit() {}
  [[nodiscard]] int32_t width() const { return 320; }
  [[nodiscard]] int32_t height() const { return 240; }
};

TEST(DriverConcept, MockSatisfiesDisplayDriverConcept) {
  static_assert(lv::drivers::DisplayDriver<MockDisplayDriver>);
}

// ── InputDriver concept ─────────────────────────────────────────────────────

struct MockInputDriver {
  void read(lv::drivers::InputState& s) { s.pressed = true; }
};

TEST(DriverConcept, MockSatisfiesInputDriverConcept) {
  static_assert(lv::drivers::InputDriver<MockInputDriver>);
}

// ── Full integration: Display + widgets + events + render ───────────────────

TEST(Integration, DisplayWithButtonClickCounter) {
  lv::tick_reset();

  int flush_count = 0;
  auto flush = [&](lv::Display&, const lv::Area&, lv::BufferView2D<Pixel>) {
    ++flush_count;
  };

  std::vector<Pixel> buf(100 * 100);
  lv::Display display(100, 100, flush);
  display.set_draw_buffers(buf);
  display.set_render_mode(lv::RenderMode::Full);

  // Create button with label
  auto& scr = display.active_screen();
  auto& btn = scr.create<lv::Button>();
  btn.set_size(80, 30).align(lv::Align::Center);

  lv::Style style;
  style.set(lv::prop::BgColor{}, lv::Color::Blue());
  btn.add_style(style);

  auto& lbl = btn.create<lv::Label>();
  lbl.set_text(std::string_view{"0"});

  int click_count = 0;
  btn.on(lv::EventCode::Clicked,
         [&](lv::Event&) {
           ++click_count;
           lbl.set_text(std::to_string(click_count));
         })
      .release();

  // Initial render
  display.refresh();
  EXPECT_GE(flush_count, 1);

  // Simulate click
  btn.send_event(lv::EventCode::Clicked);
  EXPECT_EQ(click_count, 1);
  EXPECT_EQ(lbl.text(), "1");

  // Another click
  btn.send_event(lv::EventCode::Clicked);
  EXPECT_EQ(click_count, 2);
  EXPECT_EQ(lbl.text(), "2");

  // Refresh and verify flush still works
  display.refresh();
  EXPECT_GE(flush_count, 2);

  lv::tick_reset();
}

// ── pkg-config / install verification ───────────────────────────────────────

TEST(Install, PkgConfigNameIsCorrect) {
  // This is a compile-time verification — the project name is "lvgl-cxx"
  // and the version string is available via the config header.
  EXPECT_EQ(std::string(LVGLCXX_VERSION_STRING), "1.0.0");
}

TEST(Install, BaselineVersion) {
  EXPECT_EQ(std::string(LVGLCXX_LVGL_BASELINE), "v9.5.0");
}

}  // namespace
