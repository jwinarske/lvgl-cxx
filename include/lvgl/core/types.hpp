// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — core/types.hpp
// Upstream LVGL baseline: v9.5.0  https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// Common enumeration and value types shared between the Object, Event and Style
// layers.  This header has NO intra-library dependencies and is safe to include
// from anywhere without risking circular includes.

#pragma once

#include <cstdint>

namespace lv {

// ── Flags (mirrors lv_obj_flag_t) ─────────────────────────────────────────────
enum class ObjFlags : uint32_t {
    None            = 0,
    Hidden          = 1u << 0,
    Clickable       = 1u << 1,
    ClickFocusable  = 1u << 2,
    Checkable       = 1u << 3,
    Scrollable      = 1u << 4,
    ScrollElastic   = 1u << 5,
    ScrollMomentum  = 1u << 6,
    ScrollOne       = 1u << 7,
    ScrollChainH    = 1u << 8,
    ScrollChainV    = 1u << 9,
    ScrollOnFocus   = 1u << 10,
    SnappableX      = 1u << 11,
    PressLock       = 1u << 12,
    EventBubble     = 1u << 13,
    GestureBubble   = 1u << 14,
    AdvHittest      = 1u << 15,
    IgnoreLayout    = 1u << 16,
    Floating        = 1u << 17,
    OverflowVisible = 1u << 18,
    EventTrickle    = 1u << 19,
    StateTrickle    = 1u << 20,
    User1           = 1u << 24,
    User2           = 1u << 25,
    User3           = 1u << 26,
    User4           = 1u << 27,
};

[[nodiscard]] constexpr ObjFlags operator|(ObjFlags a, ObjFlags b) noexcept {
    return static_cast<ObjFlags>(static_cast<uint32_t>(a) |
                                 static_cast<uint32_t>(b));
}
[[nodiscard]] constexpr ObjFlags operator&(ObjFlags a, ObjFlags b) noexcept {
    return static_cast<ObjFlags>(static_cast<uint32_t>(a) &
                                 static_cast<uint32_t>(b));
}
[[nodiscard]] constexpr ObjFlags operator~(ObjFlags a) noexcept {
    return static_cast<ObjFlags>(~static_cast<uint32_t>(a));
}
constexpr ObjFlags& operator|=(ObjFlags& a, ObjFlags b) noexcept {
    return a = a | b;
}
constexpr ObjFlags& operator&=(ObjFlags& a, ObjFlags b) noexcept {
    return a = a & b;
}

// ── States (mirrors lv_state_t) ───────────────────────────────────────────────
enum class ObjState : uint16_t {
    Default  = 0x0000,
    Checked  = 0x0001,
    Focused  = 0x0002,
    FocusKey = 0x0004,
    Edited   = 0x0008,
    Hovered  = 0x0010,
    Pressed  = 0x0020,
    Scrolled = 0x0040,
    Disabled = 0x0080,
    User1    = 0x1000,
    User2    = 0x2000,
    User3    = 0x4000,
    User4    = 0x8000,
    Any      = 0xFFFF,
};

[[nodiscard]] constexpr ObjState operator|(ObjState a, ObjState b) noexcept {
    return static_cast<ObjState>(static_cast<uint16_t>(a) |
                                 static_cast<uint16_t>(b));
}
[[nodiscard]] constexpr ObjState operator&(ObjState a, ObjState b) noexcept {
    return static_cast<ObjState>(static_cast<uint16_t>(a) &
                                 static_cast<uint16_t>(b));
}
[[nodiscard]] constexpr ObjState operator~(ObjState a) noexcept {
    return static_cast<ObjState>(
        static_cast<uint16_t>(~static_cast<uint16_t>(a)));
}
constexpr ObjState& operator|=(ObjState& a, ObjState b) noexcept {
    return a = a | b;
}
constexpr ObjState& operator&=(ObjState& a, ObjState b) noexcept {
    return a = a & b;
}

// ── Alignment (mirrors lv_align_t) ────────────────────────────────────────────
enum class Align : uint8_t {
    Default = 0,
    TopLeft,
    TopMid,
    TopRight,
    BottomLeft,
    BottomMid,
    BottomRight,
    LeftMid,
    Center,
    RightMid,
    OutTopLeft,
    OutTopMid,
    OutTopRight,
    OutBottomLeft,
    OutBottomMid,
    OutBottomRight,
    OutLeftTop,
    OutLeftMid,
    OutLeftBottom,
    OutRightTop,
    OutRightMid,
    OutRightBottom,
};

// ── Scroll helpers ────────────────────────────────────────────────────────────
enum class ScrollbarMode : uint8_t { Off, On, Active, Auto };
enum class Dir : uint8_t {
    None   = 0,
    Left   = 1,
    Right  = 2,
    Top    = 4,
    Bottom = 8,
    Hor    = 3,
    Ver    = 12,
    All    = 15,
};
enum class AnimEnable : uint8_t { Off = 0, On = 1 };

// Sentinel size values (mirrors LV_SIZE_CONTENT / LV_PCT)
inline constexpr int32_t SizeContent = 0x7FFFFFF0;
[[nodiscard]] inline constexpr int32_t SizePct(int32_t pct) noexcept {
    return static_cast<int32_t>(0x80000000) | pct;
}

// ── Style part (mirrors lv_part_t) ────────────────────────────────────────────
enum class Part : uint32_t {
    Main      = 0x000000,
    Scrollbar = 0x010000,
    Indicator = 0x020000,
    Knob      = 0x030000,
    Selected  = 0x040000,
    Items     = 0x050000,
    Cursor    = 0x060000,
    Any       = 0x0F0000,
};

// ── Style selector (Part × ObjState scope) ────────────────────────────────────
// Mirrors lv_style_selector_t but as a plain value type.
struct StyleSelector {
    Part     part  = Part::Main;
    ObjState state = ObjState::Default;

    // Pre-built common selectors
    static const StyleSelector Default;
    static const StyleSelector Pressed;
    static const StyleSelector Focused;
    static const StyleSelector Checked;
    static const StyleSelector Disabled;
};

inline const StyleSelector StyleSelector::Default  = {};
inline const StyleSelector StyleSelector::Pressed  = {Part::Main, ObjState::Pressed};
inline const StyleSelector StyleSelector::Focused  = {Part::Main, ObjState::Focused};
inline const StyleSelector StyleSelector::Checked  = {Part::Main, ObjState::Checked};
inline const StyleSelector StyleSelector::Disabled = {Part::Main, ObjState::Disabled};

}  // namespace lv
