// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — src/core/event.cpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// Phase 3: Event class implementation.
// Note: EventHandle is defined in object.cpp because its Impl is coupled to
//       Object::Impl (handler list + validity token).

#include "lvgl/core/event.hpp"
#include "lvgl/core/object.hpp"

namespace lv {

// ── Event
// ─────────────────────────────────────────────────────────────────────

Event::Event(Object& target, EventCode code, void* param) noexcept
    : target_(&target), current_target_(&target), code_(code), param_(param) {}

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

// Parameter accessors — most are only valid for specific EventCode values.
// param_ is cast to the appropriate type per event code.

uint32_t Event::key() const {
  // EventCode::Key stores the key code as a uintptr_t cast through void*.
  if (code_ != EventCode::Key)
    return 0;
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(param_));
}
int32_t Event::rotary_diff() const {
  if (code_ != EventCode::Rotary)
    return 0;
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  return static_cast<int32_t>(reinterpret_cast<intptr_t>(param_));
}
// NOLINTBEGIN(readability-convert-member-functions-to-static)
const InputDevice* Event::indev() const {
  return nullptr;
}
Layer* Event::layer() const {
  return nullptr;
}
ObjState Event::prev_state() const {
  return ObjState::Default;
}
const Area* Event::old_size() const {
  return nullptr;
}
Animation* Event::scroll_anim() const {
  return nullptr;
}
// NOLINTEND(readability-convert-member-functions-to-static)

void Event::stop() noexcept {
  stopped_ = true;
}
void Event::stop_bubbling() noexcept {
  bubble_stopped_ = true;
}
bool Event::is_stopped() const noexcept {
  return stopped_;
}
bool Event::is_bubbling_stopped() const noexcept {
  return bubble_stopped_;
}

void Event::set_ext_draw_size(int32_t /*size*/) noexcept {}
void Event::set_cover_res(CoverResult /*res*/) noexcept {}

}  // namespace lv
