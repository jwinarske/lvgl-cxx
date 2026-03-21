// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Joel Winarske
//
// Phase 4 unit tests: Animation and Tick

#include <gtest/gtest.h>

#include "lvgl/core/animation.hpp"
#include "lvgl/tick/tick.hpp"

namespace {

class AnimationTest : public ::testing::Test {
 protected:
  void SetUp() override {
    lv::anim_clear_all();
    lv::tick_reset();
  }
  void TearDown() override {
    lv::anim_clear_all();
    lv::tick_reset();
  }
};

TEST_F(AnimationTest, TickIncrementAndGet) {
  EXPECT_EQ(lv::tick_get(), 0u);

  lv::tick_increment(10);
  EXPECT_EQ(lv::tick_get(), 10u);

  lv::tick_increment(5);
  EXPECT_EQ(lv::tick_get(), 15u);
}

TEST_F(AnimationTest, TickElapsed) {
  lv::tick_increment(100);
  uint32_t prev = lv::tick_get();

  lv::tick_increment(50);
  EXPECT_EQ(lv::tick_elapsed(prev), 50u);
}

TEST_F(AnimationTest, TickReset) {
  lv::tick_increment(100);
  lv::tick_reset();
  EXPECT_EQ(lv::tick_get(), 0u);
}

TEST_F(AnimationTest, BuilderSetRange) {
  lv::Animation anim;
  anim.set_range(0, 200)
      .set_duration(std::chrono::milliseconds{500})
      .set_easing(lv::AnimPath::Linear);

  // Builder returns *this for chaining — just verify it compiles/runs
}

TEST_F(AnimationTest, StartAndRun) {
  int32_t last_value = -1;

  lv::Animation anim;
  anim.set_range(0, 100)
      .set_duration(std::chrono::milliseconds{100})
      .set_easing(lv::AnimPath::Linear)
      .set_exec([&](int32_t v) { last_value = v; });

  [[maybe_unused]] auto handle = anim.start();

  // Advance tick halfway
  lv::tick_increment(50);
  lv::anim_tick();

  // Value should be approximately 50 (linear interpolation)
  EXPECT_GE(last_value, 40);
  EXPECT_LE(last_value, 60);

  // Advance to end
  lv::tick_increment(60);
  lv::anim_tick();

  EXPECT_EQ(last_value, 100);
}

TEST_F(AnimationTest, EarlyApply) {
  int32_t last_value = -1;

  lv::Animation anim;
  anim.set_range(42, 100)
      .set_duration(std::chrono::milliseconds{100})
      .set_early_apply(true)
      .set_exec([&](int32_t v) { last_value = v; });

  [[maybe_unused]] auto handle = anim.start();

  // Early apply should set the start value immediately
  EXPECT_EQ(last_value, 42);
}

TEST_F(AnimationTest, OnComplete) {
  bool completed = false;

  lv::Animation anim;
  anim.set_range(0, 100)
      .set_duration(std::chrono::milliseconds{50})
      .set_exec([](int32_t) {})
      .set_on_complete([&]() { completed = true; });

  [[maybe_unused]] auto handle = anim.start();

  lv::tick_increment(60);
  lv::anim_tick();

  EXPECT_TRUE(completed);
}

TEST_F(AnimationTest, DelayPostponesStart) {
  int32_t last_value = -1;

  lv::Animation anim;
  anim.set_range(0, 100)
      .set_duration(std::chrono::milliseconds{100})
      .set_delay(std::chrono::milliseconds{50})
      .set_exec([&](int32_t v) { last_value = v; });

  [[maybe_unused]] auto handle = anim.start();

  // At tick 30, still in delay — exec should not have been called
  lv::tick_increment(30);
  lv::anim_tick();
  EXPECT_EQ(last_value, -1);

  // At tick 100 (50 delay + 50 anim), should be halfway
  lv::tick_increment(70);
  lv::anim_tick();
  EXPECT_GE(last_value, 40);
  EXPECT_LE(last_value, 60);
}

TEST_F(AnimationTest, DeleteByHandle) {
  int32_t call_count = 0;

  lv::Animation anim;
  anim.set_range(0, 100)
      .set_duration(std::chrono::milliseconds{100})
      .set_exec([&](int32_t) { ++call_count; });

  [[maybe_unused]] auto handle = anim.start();

  lv::tick_increment(10);
  lv::anim_tick();
  EXPECT_EQ(call_count, 1);

  lv::Animation::delete_by_handle(handle);

  lv::tick_increment(10);
  lv::anim_tick();
  EXPECT_EQ(call_count, 1);  // Should not have been called again
}

TEST_F(AnimationTest, StepEasing) {
  int32_t last_value = -1;

  lv::Animation anim;
  anim.set_range(0, 100)
      .set_duration(std::chrono::milliseconds{100})
      .set_easing(lv::AnimPath::Step)
      .set_exec([&](int32_t v) { last_value = v; });

  [[maybe_unused]] auto handle = anim.start();

  // Before halfway: should be 0
  lv::tick_increment(40);
  lv::anim_tick();
  EXPECT_EQ(last_value, 0);

  // After halfway: should be 100
  lv::tick_increment(20);
  lv::anim_tick();
  EXPECT_EQ(last_value, 100);
}

TEST(Ticker, UpdateIncrementsTick) {
  lv::tick_reset();
  lv::Ticker ticker;

  // Just verify it doesn't crash; actual timing is hard to test
  ticker.update();
  // Tick might be 0 or very small depending on test speed
}

TEST(AnimationTimeline, AddAndPlayDoesNotCrash) {
  lv::tick_reset();

  lv::AnimationTimeline timeline;

  lv::Animation a1;
  a1.set_range(0, 100)
      .set_duration(std::chrono::milliseconds{50})
      .set_exec([](int32_t) {});

  lv::Animation a2;
  a2.set_range(0, 200)
      .set_duration(std::chrono::milliseconds{50})
      .set_exec([](int32_t) {});

  timeline.add(0, std::move(a1));
  timeline.add(100, std::move(a2));

  timeline.play();

  lv::tick_increment(50);
  lv::anim_tick();

  timeline.stop();
  lv::tick_reset();
}

}  // namespace
