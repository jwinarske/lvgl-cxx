// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Joel Winarske
//
// Phase 4 unit tests: Display

#include <gtest/gtest.h>

#include "lvgl/core/display.hpp"
#include "lvgl/core/object.hpp"
#include "lvgl/core/screen.hpp"
#include "lvgl/core/style.hpp"

namespace {

using Pixel = lv::Display::Pixel;

// Test widget — Object ctor is protected, so we need a public subclass.
class TestWidget : public lv::Object {
 public:
  explicit TestWidget(Object* parent) : Object(parent) {}
};

// Helper: a flush callback that records calls
struct FlushRecorder {
  int call_count = 0;
  lv::Area last_area;
  std::vector<Pixel> last_pixels;

  void operator()(lv::Display& /*disp*/,
                  const lv::Area& area,
                  lv::BufferView2D<Pixel> view) {
    ++call_count;
    last_area = area;
    last_pixels.clear();
    for (int32_t r = 0; r < view.rows; ++r)
      for (int32_t c = 0; c < view.cols; ++c)
        last_pixels.push_back(view(r, c));
  }
};

TEST(Display, ConstructionAndResolution) {
  FlushRecorder recorder;
  lv::Display disp(320, 240, std::ref(recorder));

  EXPECT_EQ(disp.horizontal_resolution(), 320);
  EXPECT_EQ(disp.vertical_resolution(), 240);
}

TEST(Display, HasDefaultScreen) {
  FlushRecorder recorder;
  lv::Display disp(320, 240, std::ref(recorder));

  // Should have an active screen
  auto& scr = disp.active_screen();
  EXPECT_EQ(scr.owner_display(), &disp);
}

TEST(Display, CreateScreen) {
  FlushRecorder recorder;
  lv::Display disp(320, 240, std::ref(recorder));

  auto& scr2 = disp.create_screen();
  EXPECT_EQ(scr2.owner_display(), &disp);
}

TEST(Display, LoadScreen) {
  FlushRecorder recorder;
  lv::Display disp(320, 240, std::ref(recorder));

  auto scr2 = std::make_unique<lv::Screen>(&disp);
  auto* raw = scr2.get();
  auto& loaded = disp.load_screen(std::move(scr2));

  EXPECT_EQ(&loaded, raw);
  EXPECT_EQ(&disp.active_screen(), raw);
}

TEST(Display, SetResolution) {
  FlushRecorder recorder;
  lv::Display disp(320, 240, std::ref(recorder));

  disp.set_resolution(640, 480);
  EXPECT_EQ(disp.horizontal_resolution(), 640);
  EXPECT_EQ(disp.vertical_resolution(), 480);
}

TEST(Display, SetRotation) {
  FlushRecorder recorder;
  lv::Display disp(320, 240, std::ref(recorder));

  disp.set_rotation(lv::DisplayRotation::Deg90);
  EXPECT_EQ(disp.rotation(), lv::DisplayRotation::Deg90);
}

TEST(Display, RefreshWithoutBufferIsNoOp) {
  FlushRecorder recorder;
  lv::Display disp(320, 240, std::ref(recorder));

  disp.set_render_mode(lv::RenderMode::Full);
  disp.refresh();

  // No buffer set — flush should not be called
  EXPECT_EQ(recorder.call_count, 0);
}

TEST(Display, RefreshCallsFlushCallback) {
  FlushRecorder recorder;
  lv::Display disp(10, 10, std::ref(recorder));

  std::vector<Pixel> buffer(10 * 10);
  disp.set_draw_buffers(buffer);

  disp.set_render_mode(lv::RenderMode::Full);
  disp.refresh();

  // Flush should have been called at least once
  EXPECT_GE(recorder.call_count, 1);
}

TEST(Display, RefreshRendersStyledObject) {
  FlushRecorder recorder;
  lv::Display disp(10, 10, std::ref(recorder));

  std::vector<Pixel> buffer(10 * 10);
  disp.set_draw_buffers(buffer);

  // Add a styled object to the screen
  auto& scr = disp.active_screen();
  auto& obj = scr.create<TestWidget>();
  obj.set_pos(2, 2);
  obj.set_size(5, 5);

  lv::Style bg_style;
  bg_style.set(lv::prop::BgColor{}, lv::Color::Red());
  obj.add_style(bg_style);

  disp.set_render_mode(lv::RenderMode::Full);
  disp.refresh();

  EXPECT_GE(recorder.call_count, 1);
  // Verify some pixels were written
  EXPECT_FALSE(recorder.last_pixels.empty());
}

TEST(Display, PartialRefreshNoOpWhenClean) {
  FlushRecorder recorder;
  lv::Display disp(10, 10, std::ref(recorder));

  std::vector<Pixel> buffer(10 * 10);
  disp.set_draw_buffers(buffer);

  // Partial mode (default) + no dirty areas = no flush
  disp.refresh();
  EXPECT_EQ(recorder.call_count, 0);
}

TEST(Display, SetRenderMode) {
  FlushRecorder recorder;
  lv::Display disp(10, 10, std::ref(recorder));

  disp.set_render_mode(lv::RenderMode::Direct);
  // Just verifying it doesn't crash
}

TEST(Display, SetAntialiasing) {
  FlushRecorder recorder;
  lv::Display disp(10, 10, std::ref(recorder));

  disp.set_antialiasing(true);
  // Verifying it doesn't crash
}

TEST(Display, SetBacklight) {
  FlushRecorder recorder;
  lv::Display disp(10, 10, std::ref(recorder));

  disp.set_backlight(128);
  // Verifying it doesn't crash
}

TEST(Display, NonCopyable) {
  static_assert(!std::is_copy_constructible_v<lv::Display>);
  static_assert(!std::is_copy_assignable_v<lv::Display>);
}

}  // namespace
