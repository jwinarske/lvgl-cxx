// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — core/group.hpp
// Upstream LVGL baseline: v9.5.0  https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// lv::Group — focus management / keyboard navigation group.
// Replaces lv_group_t and the lv_group_* free-function API.

#pragma once

#include <cstdint>
#include <functional>

namespace lv {

class Object;

// ── Refocus policy ────────────────────────────────────────────────────────────
enum class RefocusPolicy : uint8_t { Next = 0, Prev = 1 };

// ── Group ─────────────────────────────────────────────────────────────────────
class Group {
public:
    Group() noexcept;
    ~Group();

    // Non-copyable
    Group(const Group&)            = delete;
    Group& operator=(const Group&) = delete;

    // Membership
    void add(Object& obj);
    void remove(Object& obj);
    void remove_all();
    void swap(Object& a, Object& b);

    // Focus traversal
    void focus_next()                     noexcept;
    void focus_prev()                     noexcept;
    void focus(Object& obj)               noexcept;
    void focus_freeze(bool en)            noexcept;

    [[nodiscard]] Object*       focused()       noexcept;
    [[nodiscard]] const Object* focused() const noexcept;

    // Navigation callbacks
    void set_on_focus_change(std::move_only_function<void(Group&)> fn);
    void set_on_edge(std::move_only_function<void(Group&, bool)> fn);

    // Policy
    void set_refocus_policy(RefocusPolicy p) noexcept;
    void set_editing(bool en)                noexcept;
    void set_wrap(bool en)                   noexcept;

    [[nodiscard]] RefocusPolicy refocus_policy() const noexcept;
    [[nodiscard]] bool          is_editing()     const noexcept;
    [[nodiscard]] bool          wrap()           const noexcept;
    [[nodiscard]] uint32_t      obj_count()      const noexcept;
    [[nodiscard]] Object*       obj_at(uint32_t index) noexcept;

    // Default group (new focus-accepting widgets are added automatically)
    void set_as_default();
    [[nodiscard]] static Group* default_group() noexcept;

    // Send a key to the focused widget
    void send_key(uint32_t key);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace lv
