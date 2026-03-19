// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — core/event.hpp
// Upstream LVGL baseline: v9.5.0  https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// Type-safe event system replacing lv_event_t / lv_event_cb_t.
//
// Circular-include note:
//   object.hpp forward-declares EventCode/EventHandle so it does NOT include
//   this header.  event.hpp forward-declares Object so it does NOT include
//   object.hpp.  Translation units that need both must include both headers
//   explicitly.

#pragma once

#include <cstdint>
#include <memory>

#include "types.hpp"  // ObjState (needed by Event::prev_state)

namespace lv {

// ── Forward declarations ──────────────────────────────────────────────────────
class Object;
class InputDevice;
class Layer;
class Area;
class Animation;

// ── Event codes (lv_event_code_t) ─────────────────────────────────────────────
enum class EventCode : uint32_t {
    // Input device
    Pressed, PressLost, ShortClicked,
    LongPressed, LongPressedRepeat,
    Clicked, Released,
    Scroll, ScrollBegin, ScrollEnd, ScrollThrowBegin,
    Gesture, Key, Rotary,
    Focused, Defocused,

    // Widget value / state
    ValueChanged, Insert, Refresh,
    Ready, Cancel, CheckedChanged,

    // Layout / draw
    Draw, DrawMain, DrawPost,
    DrawTaskAdded, RefreshExtDrawSize,
    GetSelfSize, HitTest, CoverCheck,

    // Lifecycle
    Created, Deleted,
    ChildCreated, ChildDeleted,
    ScreenLoaded, ScreenUnloaded,
    ScreenLoadStart, ScreenUnloadStart,
    SizeChanged, StyleChanged,
    LayoutChanged, StateChanged,

    // User-defined range
    User1 = 0x100,
    User2, User3, User4,
};

// ── Cover result (used with EventCode::CoverCheck) ────────────────────────────
enum class CoverResult : uint8_t { Cover = 0, NotCover = 1, Masked = 2 };

// ── Event ─────────────────────────────────────────────────────────────────────
class Event {
public:
    [[nodiscard]] Object&       target()         noexcept;
    [[nodiscard]] const Object& target()   const noexcept;
    [[nodiscard]] Object&       current_target()       noexcept;
    [[nodiscard]] const Object& current_target() const noexcept;

    // Typed target cast — asserts in debug if wrong type
    template <std::derived_from<Object> T>
    [[nodiscard]] T& target() {
        return static_cast<T&>(target());
    }

    [[nodiscard]] EventCode code() const noexcept { return code_; }

    // Parameter accessors (valid only for the matching event code)
    [[nodiscard]] uint32_t           key()         const;  // Key
    [[nodiscard]] int32_t            rotary_diff()  const;  // Rotary
    [[nodiscard]] const InputDevice* indev()        const;  // pointer/encoder
    [[nodiscard]] Layer*             layer()        const;  // draw events
    [[nodiscard]] ObjState           prev_state()   const;  // StateChanged
    [[nodiscard]] const Area*        old_size()     const;  // SizeChanged
    [[nodiscard]] Animation*         scroll_anim()  const;  // ScrollBegin

    void stop()          noexcept;
    void stop_bubbling() noexcept;
    [[nodiscard]] bool is_stopped()          const noexcept;
    [[nodiscard]] bool is_bubbling_stopped() const noexcept;

    void set_ext_draw_size(int32_t size) noexcept;
    void set_cover_res(CoverResult res)  noexcept;

private:
    friend class Object;
    Event(Object& target, EventCode code, void* param) noexcept;

    Object*   target_         = nullptr;
    Object*   current_target_ = nullptr;
    EventCode code_           = EventCode::Created;
    void*     param_          = nullptr;
    bool      stopped_        = false;
    bool      bubble_stopped_ = false;
};

// ── RAII handler token ────────────────────────────────────────────────────────
// Returned by Object::on(…).  Destroying the handle removes the handler.
// Call release() to detach from RAII (handler lives until the owning object
// is deleted).
class [[nodiscard]] EventHandle {
public:
    EventHandle() noexcept;  // defined in event.cpp after Impl is complete
    ~EventHandle();

    EventHandle(EventHandle&&) noexcept;
    EventHandle& operator=(EventHandle&&) noexcept;

    // Detach: handler persists until the owning object is deleted
    void release() noexcept;
    // Explicitly remove the handler now
    void remove() noexcept;

    [[nodiscard]] bool valid()  const noexcept;
    explicit operator bool()   const noexcept { return valid(); }

private:
    friend class Object;
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace lv
