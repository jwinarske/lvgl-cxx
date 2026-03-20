// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — tick/tick.hpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// Monotonic tick counter for the animation and refresh subsystems.
// The user must call tick_increment() periodically (e.g. from a timer ISR
// or a 1 ms OS tick) to advance the library clock.

#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>

namespace lv {

namespace detail {
inline std::atomic<uint32_t> tick_counter{0};
}  // namespace detail

// Call periodically to advance the library's internal clock.
inline void tick_increment(uint32_t ms) noexcept {
  detail::tick_counter.fetch_add(ms, std::memory_order_relaxed);
}

// Get the current tick count in milliseconds.
[[nodiscard]] inline uint32_t tick_get() noexcept {
  return detail::tick_counter.load(std::memory_order_relaxed);
}

// Get elapsed time since a reference tick.
[[nodiscard]] inline uint32_t tick_elapsed(uint32_t prev) noexcept {
  return tick_get() - prev;
}

// Reset tick counter (for testing).
inline void tick_reset() noexcept {
  detail::tick_counter.store(0, std::memory_order_relaxed);
}

// ── Ticker ──────────────────────────────────────────────────────────────────
// RAII helper that uses std::chrono steady_clock for automatic tick updates.
// Suitable for hosted platforms; embedded targets use manual tick_increment().

class Ticker {
 public:
  Ticker() noexcept : last_(std::chrono::steady_clock::now()) {}

  // Call to update tick counter based on wall-clock time elapsed since
  // last update() call.
  void update() noexcept {
    auto now = std::chrono::steady_clock::now();
    auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(now - last_);
    if (elapsed.count() > 0) {
      tick_increment(static_cast<uint32_t>(elapsed.count()));
      last_ = now;
    }
  }

 private:
  std::chrono::steady_clock::time_point last_;
};

}  // namespace lv
