// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — src/core/style.cpp
// Upstream LVGL baseline: v9.5.0  https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// Phase 2: Style copy/move/destructor and StyleSheet cascade resolution.
//
// Cascade algorithm (mirrors lv_obj_get_style_prop in lv_obj_style.c):
//  • Walk the entry list from the end (last-added = highest priority for ties).
//  • An entry matches when:
//      1. entry.sel.part == query.part
//      2. (entry.sel.state & query.state) == entry.sel.state
//         (all state bits required by the entry are active in the query)
//  • Among all matching entries, the one with the highest std::popcount of
//    state bits wins (most-specific state wins).  On ties the last-added entry
//    wins (reverse iteration ensures this).

#include "lvgl/core/style.hpp"

#include <algorithm>
#include <bit>  // std::popcount

namespace lv {

// ── Easing implementations (transition.hpp declarations) ─────────────────────

float Easing::EaseIn(float t) noexcept {
    return t * t;
}
float Easing::EaseOut(float t) noexcept {
    return t * (2.0f - t);
}
float Easing::EaseInOut(float t) noexcept {
    return t < 0.5f ? 2.0f * t * t
                    : -1.0f + (4.0f - 2.0f * t) * t;
}
float Easing::Overshoot(float t) noexcept {
    // Cubic overshoot — slight bounce past target then settle
    constexpr float c1 = 1.70158f;
    constexpr float c3 = c1 + 1.0f;
    const float tm1 = t - 1.0f;
    return 1.0f + c3 * tm1 * tm1 * tm1 + c1 * tm1 * tm1;
}

// ── Style ─────────────────────────────────────────────────────────────────────

Style::Style(const Style& other)     = default;
Style& Style::operator=(const Style& other) = default;
Style::Style(Style&& other) noexcept = default;
Style& Style::operator=(Style&& other) noexcept = default;
Style::~Style() noexcept             = default;

// ── StyleSheet ────────────────────────────────────────────────────────────────

void StyleSheet::add(const Style& s, StyleSelector sel) {
    entries_.push_back({&s, sel});
}

void StyleSheet::remove(const Style& s, StyleSelector sel) {
    auto it = std::remove_if(entries_.begin(), entries_.end(),
                             [&](const Entry& e) {
                                 return e.style == &s &&
                                        e.sel.part  == sel.part &&
                                        e.sel.state == sel.state;
                             });
    entries_.erase(it, entries_.end());
}

void StyleSheet::remove_all() noexcept {
    entries_.clear();
}

StyleValue StyleSheet::resolve(uint16_t prop_id,
                                Part     part,
                                ObjState state) const noexcept {
    StyleValue best = std::monostate{};
    int        best_popcount = -1;

    // Walk in reverse so last-added wins on popcount ties.
    for (auto it = entries_.rbegin(); it != entries_.rend(); ++it) {
        const Entry& e = *it;

        // 1. Part must match exactly
        if (e.sel.part != part) continue;

        // 2. Entry's state requirements must all be satisfied by query state
        const auto entry_state = static_cast<uint16_t>(e.sel.state);
        const auto query_state = static_cast<uint16_t>(state);
        if ((entry_state & query_state) != entry_state) continue;

        // 3. The style must actually have this property set
        StyleValue v = e.style->get_raw(prop_id);
        if (std::holds_alternative<std::monostate>(v)) continue;

        // 4. Prefer the most-specific (highest popcount) matching entry.
        //    Ties go to last-added (reverse iteration handles this).
        const int pc = std::popcount(entry_state);
        if (pc > best_popcount) {
            best_popcount = pc;
            best          = std::move(v);
        }
    }
    return best;
}

}  // namespace lv
