// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — src/core/event.cpp
// Upstream LVGL baseline: v9.5.0  https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// Phase 1 stubs for EventHandle and Event.
// Full implementation arrives in Phase 3 (event dispatch, bubbling, RAII token).

#include "lvgl/core/event.hpp"
#include "lvgl/core/object.hpp"

namespace lv {

// ── EventHandle::Impl ─────────────────────────────────────────────────────────
// Phase 3 will populate this with a back-pointer to the owning Object and a
// handler ID so that RAII removal works correctly.

struct EventHandle::Impl {
    // placeholder — filled in Phase 3
};

// ── EventHandle ───────────────────────────────────────────────────────────────

EventHandle::EventHandle() noexcept = default;
EventHandle::~EventHandle() = default;

EventHandle::EventHandle(EventHandle&&) noexcept = default;
EventHandle& EventHandle::operator=(EventHandle&&) noexcept = default;

void EventHandle::release() noexcept {
    impl_.reset();
}
void EventHandle::remove() noexcept {
    impl_.reset();
}
bool EventHandle::valid() const noexcept {
    return impl_ != nullptr;
}

// ── Event ─────────────────────────────────────────────────────────────────────

Event::Event(Object& target, EventCode code, void* param) noexcept
    : target_(&target),
      current_target_(&target),
      code_(code),
      param_(param) {}

Object& Event::target() noexcept {
    return *target_;
}
const Object& Event::target() const noexcept {
    return *target_;
}
Object& Event::current_target() noexcept {
    return *current_target_;
}
const Object& Event::current_target() const noexcept {
    return *current_target_;
}

uint32_t           Event::key()         const { return 0; }
int32_t            Event::rotary_diff() const { return 0; }
const InputDevice* Event::indev()       const { return nullptr; }
Layer*             Event::layer()       const { return nullptr; }
ObjState           Event::prev_state()  const { return ObjState::Default; }
const Area*        Event::old_size()    const { return nullptr; }
Animation*         Event::scroll_anim() const { return nullptr; }

void Event::stop()          noexcept { stopped_        = true; }
void Event::stop_bubbling() noexcept { bubble_stopped_ = true; }
bool Event::is_stopped()          const noexcept { return stopped_; }
bool Event::is_bubbling_stopped() const noexcept { return bubble_stopped_; }

void Event::set_ext_draw_size(int32_t /*size*/) noexcept {}
void Event::set_cover_res(CoverResult /*res*/)  noexcept {}

}  // namespace lv
