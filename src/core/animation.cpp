// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — src/core/animation.cpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// Phase 4: Animation engine implementation.

#include "lvgl/core/animation.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <vector>

#include "lvgl/tick/tick.hpp"

namespace lv {

// ── Global animation list ───────────────────────────────────────────────────

namespace {

struct RunningAnim {
  uint64_t id{};
  int32_t start{};
  int32_t end{};
  uint32_t duration_ms{};
  uint32_t delay_ms{};
  uint32_t repeat_count{};
  uint32_t repeat_delay_ms{};
  bool playback{};
  uint32_t pb_duration_ms{};
  uint32_t pb_delay_ms{};
  bool early_apply{};
  AnimPath easing{};
  EasingFn custom_easing;
  Animation::ExecFn exec_fn;
  Animation::CompleteFn complete_fn;

  uint32_t start_tick{};
  uint32_t current_repeat{};
  bool in_playback{};
  bool finished{};
};

std::vector<RunningAnim>& anim_list() {
  static std::vector<RunningAnim> list;
  return list;
}

uint64_t next_id() {
  static uint64_t counter = 0;
  return ++counter;
}

int32_t apply_easing(AnimPath easing,
                     const EasingFn& custom,
                     int32_t start,
                     int32_t end,
                     uint32_t elapsed,
                     uint32_t duration) {
  if (duration == 0)
    return end;

  if (custom) {
    return custom(start, end, elapsed, duration);
  }

  // Normalized progress 0..1024
  const auto t =
      static_cast<int32_t>(std::min(elapsed, duration) * 1024u / duration);
  int32_t path = 0;

  switch (easing) {
    case AnimPath::Linear:
      path = t;
      break;
    case AnimPath::EaseIn: {
      // Quadratic ease-in
      path = static_cast<int32_t>(static_cast<int64_t>(t) * t / 1024);
      break;
    }
    case AnimPath::EaseOut: {
      // Quadratic ease-out
      path = static_cast<int32_t>(2 * t - static_cast<int64_t>(t) * t / 1024);
      break;
    }
    case AnimPath::EaseInOut: {
      if (t < 512) {
        path = static_cast<int32_t>(static_cast<int64_t>(t) * t * 2 /
                                    (1024 * 1024) * 1024);
      } else {
        const int32_t t2 = t - 1024;
        path = static_cast<int32_t>(1024 - static_cast<int64_t>(t2) * t2 * 2 /
                                               (1024 * 1024) * 1024);
      }
      break;
    }
    case AnimPath::Overshoot: {
      // Simple overshoot
      if (t < 768) {
        path = static_cast<int32_t>(static_cast<int64_t>(t) * 1331 / 1024);
      } else {
        path = 1024 + static_cast<int32_t>((1024 - t) *
                                           static_cast<int64_t>(1024 - t) * 3 /
                                           (256 * 256) * 256 / 1024);
        path = std::min(path, 1024);
      }
      break;
    }
    case AnimPath::Bounce: {
      if (t >= 1024)
        path = 1024;
      else if (t < 364)
        path = static_cast<int32_t>(static_cast<int64_t>(t) * t * 7563 /
                                    (1024 * 1024));
      else if (t < 727)
        path = static_cast<int32_t>(756 + static_cast<int64_t>(t - 545) *
                                              (t - 545) * 7563 / (1024 * 1024));
      else
        path = static_cast<int32_t>(945 + static_cast<int64_t>(t - 876) *
                                              (t - 876) * 7563 / (1024 * 1024));
      path = std::min(path, 1024);
      break;
    }
    case AnimPath::Step:
      path = (t >= 512) ? 1024 : 0;
      break;
  }

  return start +
         static_cast<int32_t>(static_cast<int64_t>(end - start) * path / 1024);
}

}  // namespace

// ── Animation ───────────────────────────────────────────────────────────────

Animation::~Animation() noexcept = default;

Animation& Animation::set_range(int32_t start, int32_t end) noexcept {
  start_ = start;
  end_ = end;
  return *this;
}

Animation& Animation::set_duration(
    std::chrono::milliseconds duration) noexcept {
  duration_ms_ = static_cast<uint32_t>(duration.count());
  return *this;
}

Animation& Animation::set_delay(std::chrono::milliseconds delay) noexcept {
  delay_ms_ = static_cast<uint32_t>(delay.count());
  return *this;
}

Animation& Animation::set_repeat_count(uint32_t count) noexcept {
  repeat_count_ = count;
  return *this;
}

Animation& Animation::set_repeat_delay(
    std::chrono::milliseconds delay) noexcept {
  repeat_delay_ = static_cast<uint32_t>(delay.count());
  return *this;
}

Animation& Animation::set_playback(
    bool en,
    std::chrono::milliseconds pb_duration,
    std::chrono::milliseconds pb_delay) noexcept {
  playback_ = en;
  pb_duration_ = static_cast<uint32_t>(pb_duration.count());
  pb_delay_ = static_cast<uint32_t>(pb_delay.count());
  return *this;
}

Animation& Animation::set_easing(AnimPath easing) noexcept {
  easing_ = easing;
  custom_easing_ = {};
  return *this;
}

Animation& Animation::set_easing(EasingFn fn) {
  custom_easing_ = std::move(fn);
  return *this;
}

Animation& Animation::set_exec(ExecFn fn) {
  exec_fn_ = std::move(fn);
  return *this;
}

Animation& Animation::set_on_complete(CompleteFn fn) {
  complete_fn_ = std::move(fn);
  return *this;
}

Animation& Animation::set_early_apply(bool en) noexcept {
  early_apply_ = en;
  return *this;
}

Animation::Handle Animation::start() {
  Handle h;
  h.id = next_id();

  RunningAnim ra;
  ra.id = h.id;
  ra.start = start_;
  ra.end = end_;
  ra.duration_ms = duration_ms_;
  ra.delay_ms = delay_ms_;
  ra.repeat_count = repeat_count_;
  ra.repeat_delay_ms = repeat_delay_;
  ra.playback = playback_;
  ra.pb_duration_ms = pb_duration_ > 0 ? pb_duration_ : duration_ms_;
  ra.pb_delay_ms = pb_delay_;
  ra.early_apply = early_apply_;
  ra.easing = easing_;
  ra.custom_easing = std::move(custom_easing_);
  ra.exec_fn = std::move(exec_fn_);
  ra.complete_fn = std::move(complete_fn_);
  ra.start_tick = tick_get();
  ra.current_repeat = 0;
  ra.in_playback = false;
  ra.finished = false;

  if (ra.early_apply && ra.exec_fn) {
    ra.exec_fn(ra.start);
  }

  anim_list().push_back(std::move(ra));
  return h;
}

void Animation::delete_by_handle(const Handle& h) noexcept {
  auto& list = anim_list();
  std::erase_if(list, [&](const auto& a) { return a.id == h.id; });
}

void anim_clear_all() noexcept {
  anim_list().clear();
}

// ── Animation tick processing ───────────────────────────────────────────────

void anim_tick() {
  const uint32_t now = tick_get();
  auto& list = anim_list();

  for (auto& a : list) {
    if (a.finished)
      continue;

    const uint32_t elapsed_total = now - a.start_tick;

    // Still in delay?
    if (elapsed_total < a.delay_ms)
      continue;

    const uint32_t elapsed = elapsed_total - a.delay_ms;

    if (!a.in_playback) {
      // Forward phase
      if (elapsed >= a.duration_ms) {
        // Forward phase complete
        if (a.exec_fn)
          a.exec_fn(a.end);

        if (a.playback) {
          a.in_playback = true;
          a.start_tick = now + a.pb_delay_ms - a.delay_ms;
        } else {
          // Check repeat
          ++a.current_repeat;
          if (a.current_repeat < a.repeat_count) {
            a.start_tick = now + a.repeat_delay_ms - a.delay_ms;
          } else {
            a.finished = true;
            if (a.complete_fn)
              a.complete_fn();
          }
        }
      } else {
        const int32_t val = apply_easing(a.easing, a.custom_easing, a.start,
                                         a.end, elapsed, a.duration_ms);
        if (a.exec_fn)
          a.exec_fn(val);
      }
    } else {
      // Playback (reverse) phase
      if (elapsed >= a.pb_duration_ms) {
        if (a.exec_fn)
          a.exec_fn(a.start);

        a.in_playback = false;
        ++a.current_repeat;
        if (a.current_repeat < a.repeat_count) {
          a.start_tick = now + a.repeat_delay_ms - a.delay_ms;
        } else {
          a.finished = true;
          if (a.complete_fn)
            a.complete_fn();
        }
      } else {
        const int32_t val = apply_easing(a.easing, a.custom_easing, a.end,
                                         a.start, elapsed, a.pb_duration_ms);
        if (a.exec_fn)
          a.exec_fn(val);
      }
    }
  }

  // Purge finished animations
  std::erase_if(list, [](const auto& a) { return a.finished; });
}

// ── AnimationTimeline ───────────────────────────────────────────────────────

struct AnimationTimeline::Impl {
  struct Entry {
    uint32_t start_time_ms;
    Animation anim;
    Animation::Handle handle;
    bool started = false;

    Entry(uint32_t t, Animation a) : start_time_ms(t), anim(std::move(a)) {}
  };
  std::vector<Entry> entries;
};

AnimationTimeline::AnimationTimeline() noexcept
    : impl_(std::make_unique<Impl>()) {}

AnimationTimeline::~AnimationTimeline() noexcept {
  stop();
}

AnimationTimeline& AnimationTimeline::add(uint32_t start_time_ms,
                                          Animation anim) {
  impl_->entries.emplace_back(start_time_ms, std::move(anim));
  return *this;
}

void AnimationTimeline::play() noexcept {
  for (auto& e : impl_->entries) {
    if (!e.started) {
      e.anim.set_delay(std::chrono::milliseconds{e.start_time_ms});
      e.handle = e.anim.start();
      e.started = true;
    }
  }
}

void AnimationTimeline::pause() noexcept {}

void AnimationTimeline::stop() noexcept {
  if (!impl_)
    return;
  for (auto& e : impl_->entries) {
    if (e.started) {
      Animation::delete_by_handle(e.handle);
      e.started = false;
    }
  }
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
uint32_t AnimationTimeline::elapsed_ms() const noexcept {
  return 0;
}

uint32_t AnimationTimeline::duration_ms() const noexcept {
  uint32_t max = 0;
  for (const auto& e : impl_->entries) {
    max = std::max(max, e.start_time_ms + e.anim.duration_ms_);
  }
  return max;
}

bool AnimationTimeline::is_playing() const noexcept {
  return std::ranges::any_of(impl_->entries,
                             [](const auto& e) { return e.started; });
}

void AnimationTimeline::set_on_complete(UniqueFunction<void()> /*fn*/) {}

}  // namespace lv
