// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — core/style.hpp
// Upstream LVGL baseline: v9.5.0  https://github.com/lvgl/lvgl/releases/tag/v9.5.0

#pragma once

#include <cstdint>
#include <flat_map>
#include <optional>
#include <variant>

#include "../misc/color.hpp"
#include "types.hpp"  // ObjState, Part, StyleSelector

namespace lv {

// Forward declarations
class Font;
struct Transition;

// ── Style value storage ───────────────────────────────────────────────────────
using StyleValue = std::variant<
    std::monostate,  // property not set
    int32_t,         // coords, enums, opacity, radii
    Color,           // color values
    ColorFilter,     // gradient descriptor
    const Font*,     // non-owning font pointer
    const void*      // non-owning image source
    >;

// ── Property tag types ────────────────────────────────────────────────────────
// Each tag encodes its expected value type via `value_type` and a stable
// integer `id`.  New properties are added here as phases are implemented.
namespace prop {

// Sizing
struct Width   { using value_type = int32_t; static constexpr uint16_t id =  1; };
struct Height  { using value_type = int32_t; static constexpr uint16_t id =  2; };
struct MinWidth  { using value_type = int32_t; static constexpr uint16_t id =  3; };
struct MinHeight { using value_type = int32_t; static constexpr uint16_t id =  4; };
struct MaxWidth  { using value_type = int32_t; static constexpr uint16_t id =  5; };
struct MaxHeight { using value_type = int32_t; static constexpr uint16_t id =  6; };

// Padding
struct PadTop    { using value_type = int32_t; static constexpr uint16_t id = 10; };
struct PadBottom { using value_type = int32_t; static constexpr uint16_t id = 11; };
struct PadLeft   { using value_type = int32_t; static constexpr uint16_t id = 12; };
struct PadRight  { using value_type = int32_t; static constexpr uint16_t id = 13; };
struct PadRow    { using value_type = int32_t; static constexpr uint16_t id = 14; };
struct PadColumn { using value_type = int32_t; static constexpr uint16_t id = 15; };

// Background
struct BgColor   { using value_type = Color;       static constexpr uint16_t id = 20; };
struct BgOpacity { using value_type = int32_t;     static constexpr uint16_t id = 21; };
struct BgGrad    { using value_type = ColorFilter; static constexpr uint16_t id = 22; };
struct BgGradDir { using value_type = int32_t;     static constexpr uint16_t id = 23; };

// Border
struct BorderColor   { using value_type = Color;   static constexpr uint16_t id = 30; };
struct BorderOpacity { using value_type = int32_t; static constexpr uint16_t id = 31; };
struct BorderWidth   { using value_type = int32_t; static constexpr uint16_t id = 32; };
struct BorderSide    { using value_type = int32_t; static constexpr uint16_t id = 33; };
struct BorderPost    { using value_type = int32_t; static constexpr uint16_t id = 34; };

// Outline
struct OutlineColor   { using value_type = Color;   static constexpr uint16_t id = 40; };
struct OutlineOpacity { using value_type = int32_t; static constexpr uint16_t id = 41; };
struct OutlineWidth   { using value_type = int32_t; static constexpr uint16_t id = 42; };
struct OutlinePad     { using value_type = int32_t; static constexpr uint16_t id = 43; };

// Shadow
struct ShadowColor   { using value_type = Color;   static constexpr uint16_t id = 50; };
struct ShadowOpacity { using value_type = int32_t; static constexpr uint16_t id = 51; };
struct ShadowWidth   { using value_type = int32_t; static constexpr uint16_t id = 52; };
struct ShadowOffsetX { using value_type = int32_t; static constexpr uint16_t id = 53; };
struct ShadowOffsetY { using value_type = int32_t; static constexpr uint16_t id = 54; };
struct ShadowSpread  { using value_type = int32_t; static constexpr uint16_t id = 55; };

// Text
struct TextColor         { using value_type = Color;       static constexpr uint16_t id = 60; };
struct TextOpacity       { using value_type = int32_t;     static constexpr uint16_t id = 61; };
struct TextFont          { using value_type = const Font*; static constexpr uint16_t id = 62; };
struct TextLetterSpacing { using value_type = int32_t;     static constexpr uint16_t id = 63; };
struct TextLineSpacing   { using value_type = int32_t;     static constexpr uint16_t id = 64; };
struct TextDecor         { using value_type = int32_t;     static constexpr uint16_t id = 65; };
struct TextAlign         { using value_type = int32_t;     static constexpr uint16_t id = 66; };

// Image
struct ImgOpacity   { using value_type = int32_t; static constexpr uint16_t id = 70; };
struct ImgRecolor   { using value_type = Color;   static constexpr uint16_t id = 71; };
struct ImgRecolorOpa { using value_type = int32_t; static constexpr uint16_t id = 72; };

// Misc
struct Radius          { using value_type = int32_t; static constexpr uint16_t id = 80; };
struct Opacity         { using value_type = int32_t; static constexpr uint16_t id = 81; };
struct ColorFilterOpa  { using value_type = int32_t; static constexpr uint16_t id = 82; };
struct AnimDuration    { using value_type = int32_t; static constexpr uint16_t id = 83; };
struct BlendMode       { using value_type = int32_t; static constexpr uint16_t id = 84; };
struct Transform       { using value_type = int32_t; static constexpr uint16_t id = 85; };
struct TransformWidth  { using value_type = int32_t; static constexpr uint16_t id = 86; };
struct TransformHeight { using value_type = int32_t; static constexpr uint16_t id = 87; };
struct TransformScaleX { using value_type = int32_t; static constexpr uint16_t id = 88; };
struct TransformScaleY { using value_type = int32_t; static constexpr uint16_t id = 89; };
struct TransformRotation { using value_type = int32_t; static constexpr uint16_t id = 90; };
struct TransformPivotX   { using value_type = int32_t; static constexpr uint16_t id = 91; };
struct TransformPivotY   { using value_type = int32_t; static constexpr uint16_t id = 92; };

}  // namespace prop

// ── StyleProperty concept ─────────────────────────────────────────────────────
template <typename P>
concept StyleProperty = requires {
    typename P::value_type;
    { P::id } -> std::convertible_to<uint16_t>;
};

// ── Style ─────────────────────────────────────────────────────────────────────
class Style {
public:
    Style() noexcept = default;
    Style(const Style&);
    Style& operator=(const Style&);
    Style(Style&&) noexcept;
    Style& operator=(Style&&) noexcept;
    ~Style() noexcept;

    // Typed setter — returns *this for chaining
    template <StyleProperty P>
    Style& set(P /*tag*/, typename P::value_type value) {
        props_[P::id] = StyleValue{std::move(value)};
        return *this;
    }

    // Typed getter — nullopt if not set
    template <StyleProperty P>
    [[nodiscard]] std::optional<typename P::value_type> get(P /*tag*/) const {
        auto it = props_.find(P::id);
        if (it == props_.end()) return std::nullopt;
        if (auto* v = std::get_if<typename P::value_type>(&it->second))
            return *v;
        return std::nullopt;
    }

    template <StyleProperty P>
    Style& remove(P /*tag*/) noexcept {
        props_.erase(P::id);
        return *this;
    }

    void                reset()  noexcept { props_.clear(); }
    [[nodiscard]] bool  empty()  const noexcept { return props_.empty(); }
    [[nodiscard]] std::size_t size() const noexcept { return props_.size(); }

private:
    // std::flat_map: sorted, contiguous, cache-friendly — requires GCC 14 / C++23
    std::flat_map<uint16_t, StyleValue> props_;
};

}  // namespace lv
