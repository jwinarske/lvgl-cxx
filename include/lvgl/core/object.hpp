// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — core/object.hpp
// Upstream LVGL baseline: v9.5.0  https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// lv::Object — base class for every widget in the tree.
//
// Design notes:
//  • Non-copyable and non-movable: objects have stable addresses once parented.
//  • Child lifetime is managed by unique_ptr stored in the parent.
//  • All mutable fields except the tree linkage live in Object::Impl (pimpl).
//  • ObjectRef<T> is defined AFTER Object so it can call validity_token()
//    on a complete Object type.  It auto-nullifies when the target is destroyed.

#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <ranges>
#include <vector>

#include "../misc/area.hpp"  // Area, Point
#include "../misc/function.hpp"  // lv::UniqueFunction
#include "style.hpp"         // Style, StyleSheet, StyleValue, StyleProperty, …
#include "types.hpp"         // ObjFlags, ObjState, Align, StyleSelector, ...

namespace lv {

// Forward declarations
class Display;
class Screen;
class Group;
class Event;
class DrawContext;
class Layer;

// EventCode and EventHandle defined in event.hpp — forward-declared here to
// break the circular dependency (event.hpp needs ObjState from types.hpp;
// object.hpp needs EventCode/EventHandle but does NOT include event.hpp).
enum class EventCode : uint32_t;
class EventHandle;

// ── Object ────────────────────────────────────────────────────────────────────
class Object {
public:
    Object(const Object&)            = delete;
    Object& operator=(const Object&) = delete;
    Object(Object&&)                 = delete;
    Object& operator=(Object&&)      = delete;
    virtual ~Object();

    // Validity token used by ObjectRef<T> — defined below after Object.
    [[nodiscard]] std::weak_ptr<bool> validity_token() const noexcept;

    // ── Tree ──────────────────────────────────────────────────────────────────
    [[nodiscard]] Object*       parent() noexcept;
    [[nodiscard]] const Object* parent() const noexcept;
    [[nodiscard]] std::size_t   child_count() const noexcept;
    [[nodiscard]] Object&       child_at(std::size_t index);
    [[nodiscard]] const Object& child_at(std::size_t index) const;

    [[nodiscard]] auto children() noexcept
        -> std::ranges::subrange<
               std::vector<std::unique_ptr<Object>>::iterator>;
    [[nodiscard]] auto children() const noexcept
        -> std::ranges::subrange<
               std::vector<std::unique_ptr<Object>>::const_iterator>;

    template <std::derived_from<Object> T, typename... Args>
    T& create(Args&&... args) {
        auto child = std::make_unique<T>(this, std::forward<Args>(args)...);
        T& ref     = *child;
        do_add_child(std::move(child));
        ref.on_create();
        return ref;
    }

    void remove_child(Object& child);
    void remove_child(std::size_t index);
    void remove_all_children();

    // ── Flags ─────────────────────────────────────────────────────────────────
    Object& add_flag(ObjFlags f) noexcept;
    Object& remove_flag(ObjFlags f) noexcept;
    Object& set_flag(ObjFlags f, bool v) noexcept;
    [[nodiscard]] bool has_flag(ObjFlags f) const noexcept;
    [[nodiscard]] bool has_flag_any(ObjFlags f) const noexcept;

    // ── States ────────────────────────────────────────────────────────────────
    Object& add_state(ObjState s) noexcept;
    Object& remove_state(ObjState s) noexcept;
    Object& set_state(ObjState s, bool v) noexcept;
    [[nodiscard]] ObjState state() const noexcept;
    [[nodiscard]] bool     has_state(ObjState s) const noexcept;

    // ── Geometry ──────────────────────────────────────────────────────────────
    Object& set_pos(int32_t x, int32_t y) noexcept;
    Object& set_x(int32_t x) noexcept;
    Object& set_y(int32_t y) noexcept;
    Object& set_size(int32_t w, int32_t h) noexcept;
    Object& set_width(int32_t w) noexcept;
    Object& set_height(int32_t h) noexcept;
    Object& align(Align a, int32_t x_ofs = 0, int32_t y_ofs = 0) noexcept;
    Object& align_to(const Object& base, Align a,
                     int32_t x_ofs = 0, int32_t y_ofs = 0) noexcept;

    [[nodiscard]] int32_t x() const noexcept;
    [[nodiscard]] int32_t y() const noexcept;
    [[nodiscard]] int32_t width() const noexcept;
    [[nodiscard]] int32_t height() const noexcept;
    [[nodiscard]] Area    bounds() const noexcept;

    // ── Scroll ────────────────────────────────────────────────────────────────
    Object& set_scrollbar_mode(ScrollbarMode m) noexcept;
    Object& set_scroll_dir(Dir d) noexcept;
    [[nodiscard]] ScrollbarMode scrollbar_mode() const noexcept;
    [[nodiscard]] Dir           scroll_dir() const noexcept;
    void scroll_to(int32_t x, int32_t y, AnimEnable anim) noexcept;
    void scroll_by(int32_t dx, int32_t dy, AnimEnable anim) noexcept;
    [[nodiscard]] int32_t scroll_x() const noexcept;
    [[nodiscard]] int32_t scroll_y() const noexcept;

    // ── Styles (Phase 2) ─────────────────────────────────────────────────────
    Object& add_style(const Style& s, StyleSelector sel = {});
    Object& remove_style(const Style& s, StyleSelector sel = {});
    Object& remove_all_styles();

    // Typed cascade resolver — queries the object's internal StyleSheet.
    // Returns nullopt if no style sets the property for this (part, state).
    template <StyleProperty P>
    [[nodiscard]] std::optional<typename P::value_type>
    resolve_style(P /*tag*/,
                  ObjState state = ObjState::Default,
                  Part     part  = Part::Main) const noexcept {
        auto v = resolve_style_value(P::id, state, part);
        if (auto* vp = std::get_if<typename P::value_type>(&v))
            return *vp;
        return std::nullopt;
    }

    // ── Events (Phase 3) ─────────────────────────────────────────────────────
    using Handler = UniqueFunction<void(Event&)>;
    [[nodiscard]] EventHandle on(EventCode code, Handler handler);
    void remove_event(EventHandle& h);
    void send_event(EventCode code, void* param = nullptr);

    // ── Layout ────────────────────────────────────────────────────────────────
    Object& mark_layout_dirty() noexcept;

    // ── Display / Screen ──────────────────────────────────────────────────────
    [[nodiscard]] Display*       display() noexcept;
    [[nodiscard]] const Display* display() const noexcept;
    [[nodiscard]] Screen*        screen() noexcept;

    void invalidate() noexcept;
    void invalidate_area(Area a) noexcept;

protected:
    explicit Object(Object* parent);

    virtual void on_create()                     {}
    virtual void on_delete()                     {}
    virtual void on_event(Event& e)              { (void)e; }
    virtual void on_draw(DrawContext& ctx)        { (void)ctx; }
    virtual void on_size_changed()               {}
    virtual void on_style_changed()              {}
    virtual void on_child_changed(Object& child) { (void)child; }

private:
    void do_add_child(std::unique_ptr<Object> child);

    // Low-level style resolver — defined in object.cpp, delegates to Impl::sheet.
    [[nodiscard]] StyleValue resolve_style_value(uint16_t prop_id,
                                                  ObjState  state,
                                                  Part      part) const noexcept;

    // Internal event dispatch: calls all matching handlers on *this only.
    // Used by send_event for both direct dispatch and bubble/trickle propagation.
    void do_dispatch_handlers(Event& e) noexcept;

    Object*                              parent_   = nullptr;
    std::vector<std::unique_ptr<Object>> children_;

    struct Impl;
    std::unique_ptr<Impl> impl_;
};

// ── ObjectRef<T> ──────────────────────────────────────────────────────────────
// Non-owning handle that auto-nullifies when the target Object is destroyed.
// Defined here — AFTER Object — so Object is complete and validity_token()
// can be called safely.
template <typename T>
class ObjectRef {
public:
    ObjectRef() noexcept = default;

    // NOLINTNEXTLINE(google-explicit-constructor)
    ObjectRef(T& obj) noexcept
        : token_(static_cast<Object&>(obj).validity_token()), ptr_(&obj) {}

    [[nodiscard]] bool valid() const noexcept {
        auto sp = token_.lock();
        return sp && *sp;
    }
    [[nodiscard]] T*       get()       noexcept { return valid() ? ptr_ : nullptr; }
    [[nodiscard]] const T* get() const noexcept { return valid() ? ptr_ : nullptr; }

    T& operator*() const { assert(valid()); return *ptr_; }
    T* operator->() const noexcept { return ptr_; }
    explicit operator bool() const noexcept { return valid(); }

private:
    std::weak_ptr<bool> token_;
    T*                  ptr_ = nullptr;
};

}  // namespace lv
