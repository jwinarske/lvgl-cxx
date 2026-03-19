// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — core/animation.hpp
// Upstream LVGL baseline: v9.5.0  https://github.com/lvgl/lvgl/releases/tag/v9.5.0

#pragma once

#include <chrono>
#include <cstdint>
#include <functional>

namespace lv {

// ── Easing functions (mirrors lv_anim_path_cb_t) ─────────────────────────────
enum class Easing : uint8_t {
    Linear,
    EaseIn, EaseOut, EaseInOut,
    Overshoot,
    Bounce,
    Step,
};

// User-supplied custom easing: maps progress [0,255] → value
using EasingFn = std::function<int32_t(int32_t start, int32_t end,
                                       uint32_t elapsed_ms,
                                       uint32_t duration_ms)>;

// ── Animation ─────────────────────────────────────────────────────────────────
class Animation {
public:
    using ExecFn     = std::move_only_function<void(int32_t value)>;
    using CompleteFn = std::move_only_function<void()>;

    Animation()  noexcept = default;
    ~Animation() noexcept;

    // Fluent builder — each setter returns *this
    Animation& set_range(int32_t start, int32_t end) noexcept;
    Animation& set_duration(std::chrono::milliseconds duration) noexcept;
    Animation& set_delay(std::chrono::milliseconds delay) noexcept;
    Animation& set_repeat_count(uint32_t count) noexcept;
    Animation& set_repeat_delay(std::chrono::milliseconds delay) noexcept;
    Animation& set_playback(bool en,
                            std::chrono::milliseconds pb_duration = {},
                            std::chrono::milliseconds pb_delay    = {}) noexcept;
    Animation& set_easing(Easing easing) noexcept;
    Animation& set_easing(EasingFn fn);
    Animation& set_exec(ExecFn fn);
    Animation& set_on_complete(CompleteFn fn);
    Animation& set_early_apply(bool en) noexcept;

    // Start the animation — returns a handle that can be used to stop it.
    // The animation lives independently of this builder object after start().
    struct Handle;
    [[nodiscard]] Handle start();

    // Convenience: create and start an animation that sets a property back
    // to its current value after playback (ping-pong)
    static Animation& get_by_handle(const Handle& h);
    static void       delete_by_handle(const Handle& h) noexcept;

    static constexpr uint32_t RepeatInfinite = 0xFFFFFFFF;

private:
    int32_t  start_       = 0;
    int32_t  end_         = 100;
    uint32_t duration_ms_ = 300;
    uint32_t delay_ms_    = 0;
    uint32_t repeat_count_= 1;
    uint32_t repeat_delay_= 0;
    bool     playback_    = false;
    uint32_t pb_duration_ = 0;
    uint32_t pb_delay_    = 0;
    bool     early_apply_ = false;
    Easing   easing_      = Easing::Linear;
    EasingFn custom_easing_;
    ExecFn   exec_fn_;
    CompleteFn complete_fn_;
};

// ── AnimationTimeline ─────────────────────────────────────────────────────────
// Plays a sequence of animations with per-entry start times.
class AnimationTimeline {
public:
    AnimationTimeline() noexcept = default;
    ~AnimationTimeline() noexcept;

    // Add an animation starting at start_time_ms relative to timeline start
    AnimationTimeline& add(uint32_t start_time_ms, Animation anim);

    void play()  noexcept;
    void pause() noexcept;
    void stop()  noexcept;

    [[nodiscard]] uint32_t elapsed_ms()  const noexcept;
    [[nodiscard]] uint32_t duration_ms() const noexcept;
    [[nodiscard]] bool     is_playing()  const noexcept;

    // Callback when all animations have finished
    void set_on_complete(std::move_only_function<void()> fn);

private:
    struct Entry;
    std::vector<Entry> entries_;
};

} // namespace lv
