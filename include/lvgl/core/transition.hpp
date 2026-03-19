// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — core/transition.hpp
// Upstream LVGL baseline: v9.5.0  https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// Style-transition descriptor — mirrors lv_style_transition_dsc_t in LVGL v9.5.0
// (src/core/lv_obj_style.c).  Specifies how a style property value animates
// when the object's state changes.
//
// Note: this is distinct from lv::Animation (core/animation.hpp) which drives
// general-purpose value animations.  Style transitions are simpler: they only
// need duration, delay and an easing path.

#pragma once

#include <bit>      // std::popcount
#include <cstdint>
#include <vector>

namespace lv {

// ── Easing function type ──────────────────────────────────────────────────────
// Maps normalised progress t ∈ [0, 1] → eased progress ∈ [0, 1].
// C++23 noexcept is part of the function type.
using TransitionEasingFn = float (*)(float t) noexcept;

// ── Built-in easing helpers ───────────────────────────────────────────────────
// Correspond to LVGL's lv_anim_path_* functions.
namespace Easing {

[[nodiscard]] constexpr float Linear(float t) noexcept { return t; }
[[nodiscard]] float EaseIn(float t) noexcept;
[[nodiscard]] float EaseOut(float t) noexcept;
[[nodiscard]] float EaseInOut(float t) noexcept;
[[nodiscard]] float Overshoot(float t) noexcept;

}  // namespace Easing

// ── HasPropId concept ─────────────────────────────────────────────────────────
// Minimal requirement for for_props(): the type only needs a uint16_t id.
// This avoids pulling in StyleProperty (which is defined in style.hpp).
template <typename P>
concept HasPropId = requires {
    { P::id } -> std::convertible_to<uint16_t>;
};

// ── Transition ────────────────────────────────────────────────────────────────
// Stored as a style property value (prop::PropTransition) on a Style object.
// When the framework detects a state change, it looks up the Transition for
// each property that changed and schedules an animated interpolation.
struct Transition {
    uint32_t           duration_ms = 300;
    uint32_t           delay_ms    = 0;
    TransitionEasingFn easing      = &Easing::Linear;
    std::vector<uint16_t> prop_ids;  ///< IDs of style properties covered

    // Variadic helper — accumulates property IDs; returns *this for chaining.
    // Usage: Transition{}.for_props<prop::BgColor, prop::Radius>()
    template <HasPropId... Ps>
    Transition& for_props() noexcept {
        (prop_ids.push_back(Ps::id), ...);
        return *this;
    }

    /// Returns true if this transition covers the given property ID.
    [[nodiscard]] bool affects(uint16_t prop_id) const noexcept {
        for (auto id : prop_ids)
            if (id == prop_id) return true;
        return false;
    }

    /// Returns true if no properties have been registered.
    [[nodiscard]] bool empty() const noexcept { return prop_ids.empty(); }
};

}  // namespace lv
