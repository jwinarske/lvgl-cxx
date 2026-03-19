// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — src/core/group.cpp
// Upstream LVGL baseline: v9.5.0  https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// Phase 3: Group implementation — focus management / keyboard navigation.
// Mirrors lv_group.c in LVGL v9.5.0 (src/core/lv_group.c).

#include "lvgl/core/group.hpp"
#include "lvgl/core/event.hpp"
#include "lvgl/core/object.hpp"
#include "lvgl/core/types.hpp"

#include <algorithm>
#include <cstdint>

namespace lv {

// ── Group::Impl ───────────────────────────────────────────────────────────────

struct Group::Impl {
    std::vector<Object*> members;    // non-owning pointers; objects must outlive Group
    int                  focused_idx = -1;  // -1 = nothing focused
    RefocusPolicy        policy      = RefocusPolicy::Next;
    bool                 editing     = false;
    bool                 wrap        = true;
    bool                 freeze      = false;

    std::move_only_function<void(Group&)>       on_focus_change;
    std::move_only_function<void(Group&, bool)> on_edge;
};

// ── Static default group ──────────────────────────────────────────────────────

static Group* s_default_group = nullptr;

// ── Constructor / Destructor ──────────────────────────────────────────────────

Group::Group() noexcept : impl_(std::make_unique<Impl>()) {}

Group::~Group() {
    if (s_default_group == this)
        s_default_group = nullptr;

    // Remove Focused state from the currently focused object (if any)
    if (impl_->focused_idx >= 0) {
        auto* obj = impl_->members[static_cast<std::size_t>(impl_->focused_idx)];
        obj->remove_state(ObjState::Focused);
    }
}

// ── Membership ────────────────────────────────────────────────────────────────

void Group::add(Object& obj) {
    // Don't add duplicates
    if (std::find(impl_->members.begin(), impl_->members.end(), &obj)
        != impl_->members.end())
        return;
    impl_->members.push_back(&obj);
}

void Group::remove(Object& obj) {
    auto it = std::find(impl_->members.begin(), impl_->members.end(), &obj);
    if (it == impl_->members.end()) return;

    const auto idx = static_cast<int>(std::distance(impl_->members.begin(), it));

    // Remove Focused state if this was the focused object
    if (idx == impl_->focused_idx)
        obj.remove_state(ObjState::Focused);

    impl_->members.erase(it);

    // Adjust focused index
    if (impl_->focused_idx < 0) return;
    if (idx < impl_->focused_idx) {
        --impl_->focused_idx;
    } else if (idx == impl_->focused_idx) {
        impl_->focused_idx = -1;
        // Re-focus using the configured policy if members remain
        if (!impl_->members.empty()) {
            if (impl_->policy == RefocusPolicy::Next)
                focus_next();
            else
                focus_prev();
        }    }
}

void Group::remove_all() {
    // Strip Focused state from the focused object before clearing
    if (impl_->focused_idx >= 0) {
        impl_->members[static_cast<std::size_t>(impl_->focused_idx)]
            ->remove_state(ObjState::Focused);
    }
    impl_->members.clear();
    impl_->focused_idx = -1;
}

void Group::swap(Object& a, Object& b) {
    auto it_a = std::find(impl_->members.begin(), impl_->members.end(), &a);
    auto it_b = std::find(impl_->members.begin(), impl_->members.end(), &b);
    if (it_a == impl_->members.end() || it_b == impl_->members.end()) return;
    std::iter_swap(it_a, it_b);
}

// ── Focus traversal ───────────────────────────────────────────────────────────

// Internal helper: apply focus to member[new_idx], notify callbacks.
// Defined before the public methods so it can be called by all three.
void Group::do_focus(int new_idx) noexcept {
    if (impl_->freeze) return;

    const int n = static_cast<int>(impl_->members.size());
    if (n == 0) return;

    // Detect edge conditions before clamping/wrapping
    const bool at_start = (new_idx <= 0);
    const bool at_end   = (new_idx >= n - 1);

    if (!impl_->wrap) {
        if (new_idx < 0)    new_idx = 0;
        if (new_idx >= n)   new_idx = n - 1;
    } else {
        new_idx = ((new_idx % n) + n) % n;
    }

    const bool hit_edge = !impl_->wrap && (at_start || at_end)
                          && new_idx != impl_->focused_idx;

    // Remove Focused state from the previously focused object
    if (impl_->focused_idx >= 0 && impl_->focused_idx < n) {
        impl_->members[static_cast<std::size_t>(impl_->focused_idx)]
            ->remove_state(ObjState::Focused);
        impl_->members[static_cast<std::size_t>(impl_->focused_idx)]
            ->send_event(EventCode::Defocused, nullptr);
    }

    impl_->focused_idx = new_idx;
    Object* obj = impl_->members[static_cast<std::size_t>(new_idx)];
    obj->add_state(ObjState::Focused);
    obj->send_event(EventCode::Focused, nullptr);

    if (impl_->on_focus_change) impl_->on_focus_change(*this);
    if (hit_edge && impl_->on_edge) impl_->on_edge(*this, at_start);
}

void Group::focus_next() noexcept {
    if (impl_->freeze || impl_->members.empty()) return;
    const int next = (impl_->focused_idx < 0) ? 0 : impl_->focused_idx + 1;
    do_focus(next);
}

void Group::focus_prev() noexcept {
    if (impl_->freeze || impl_->members.empty()) return;
    const int n    = static_cast<int>(impl_->members.size());
    const int prev = (impl_->focused_idx < 0) ? n - 1 : impl_->focused_idx - 1;
    do_focus(prev);
}

void Group::focus(Object& obj) noexcept {
    auto it = std::find(impl_->members.begin(), impl_->members.end(), &obj);
    if (it == impl_->members.end()) return;
    const int idx = static_cast<int>(
        std::distance(impl_->members.begin(), it));
    do_focus(idx);
}

void Group::focus_freeze(bool en) noexcept {
    impl_->freeze = en;
}

// ── Getters ───────────────────────────────────────────────────────────────────

Object* Group::focused() noexcept {
    if (impl_->focused_idx < 0 ||
        impl_->focused_idx >= static_cast<int>(impl_->members.size()))
        return nullptr;
    return impl_->members[static_cast<std::size_t>(impl_->focused_idx)];
}
const Object* Group::focused() const noexcept {
    return const_cast<Group*>(this)->focused();
}

// ── Policy / Options ──────────────────────────────────────────────────────────

void Group::set_on_focus_change(std::move_only_function<void(Group&)> fn) {
    impl_->on_focus_change = std::move(fn);
}
void Group::set_on_edge(std::move_only_function<void(Group&, bool)> fn) {
    impl_->on_edge = std::move(fn);
}
void Group::set_refocus_policy(RefocusPolicy p) noexcept { impl_->policy  = p; }
void Group::set_editing(bool en)                noexcept { impl_->editing = en; }
void Group::set_wrap(bool en)                   noexcept { impl_->wrap    = en; }

RefocusPolicy Group::refocus_policy() const noexcept { return impl_->policy;  }
bool          Group::is_editing()     const noexcept { return impl_->editing; }
bool          Group::wrap()           const noexcept { return impl_->wrap;    }
uint32_t      Group::obj_count()      const noexcept {
    return static_cast<uint32_t>(impl_->members.size());
}
Object* Group::obj_at(uint32_t index) noexcept {
    if (index >= impl_->members.size()) return nullptr;
    return impl_->members[index];
}

// ── Default group ─────────────────────────────────────────────────────────────

void Group::set_as_default() {
    s_default_group = this;
}
Group* Group::default_group() noexcept {
    return s_default_group;
}

// ── Send key ──────────────────────────────────────────────────────────────────

void Group::send_key(uint32_t key) {
    if (auto* obj = focused()) {
        // Encode the key value as a void* (matching Event::key() decoding)
        obj->send_event(EventCode::Key,
                        reinterpret_cast<void*>(static_cast<uintptr_t>(key)));
    }
}

}  // namespace lv
