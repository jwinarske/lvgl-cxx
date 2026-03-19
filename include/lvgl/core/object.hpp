// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — core/object.hpp
// Upstream LVGL baseline: v9.5.0  https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// lv::Object — base class for every widget in the tree.

#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <ranges>
#include <span>
#include <string_view>
#include <vector>

#include "../misc/area.hpp"
#include "../misc/color.hpp"
#include "event.hpp"    // EventCode, EventHandle, Event (forward-declared)
#include "style.hpp"    // Style, StyleSelector (forward-declared)

namespace lv {

// Forward declarations
class Display;
class Screen;
class Group;
class Style;
class StyleSelector;
class Event;
class EventHandle;
class DrawContext;
class Layer;

// ── Flags ─────────────────────────────────────────────────────────────────────
enum class ObjFlags : uint32_t {
    None              = 0,
    Hidden            = 1u << 0,
    Clickable         = 1u << 1,
    ClickFocusable    = 1u << 2,
    Checkable         = 1u << 3,
    Scrollable        = 1u << 4,
    ScrollElastic     = 1u << 5,
    ScrollMomentum    = 1u << 6,
    ScrollOne         = 1u << 7,
    ScrollChainH      = 1u << 8,
    ScrollChainV      = 1u << 9,
    ScrollOnFocus     = 1u << 10,
    SnappableX        = 1u << 11,
    PressLock         = 1u << 12,
    EventBubble       = 1u << 13,
    GestureBubble     = 1u << 14,
    AdvHittest        = 1u << 15,
    IgnoreLayout      = 1u << 16,
    Floating          = 1u << 17,
    OverflowVisible   = 1u << 18,
    EventTrickle      = 1u << 19,
    StateTrickle      = 1u << 20,
    User1             = 1u << 24,
    User2             = 1u << 25,
    User3             = 1u << 26,
    User4             = 1u << 27,
};
[[nodiscard]] constexpr ObjFlags operator|(ObjFlags a, ObjFlags b) noexcept {
    return static_cast<ObjFlags>(
        static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}
[[nodiscard]] constexpr ObjFlags operator&(ObjFlags a, ObjFlags b) noexcept {
    return static_cast<ObjFlags>(
        static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
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

// ── States ────────────────────────────────────────────────────────────────────
enum class ObjState : uint16_t {
    Default   = 0x0000,
    Checked   = 0x0001,
    Focused   = 0x0002,
    FocusKey  = 0x0004,
    Edited    = 0x0008,
    Hovered   = 0x0010,
    Pressed   = 0x0020,
    Scrolled  = 0x0040,
    Disabled  = 0x0080,
    User1     = 0x1000,
    User2     = 0x2000,
    User3     = 0x4000,
    User4     = 0x8000,
    Any       = 0xFFFF,
};
[[nodiscard]] constexpr ObjState operator|(ObjState a, ObjState b) noexcept {
    return static_cast<ObjState>(
        static_cast<uint16_t>(a) | static_cast<uint16_t>(b));
}
[[nodiscard]] constexpr ObjState operator&(ObjState a, ObjState b) noexcept {
    return static_cast<ObjState>(
        static_cast<uint16_t>(a) & static_cast<uint16_t>(b));
}
constexpr ObjState& operator|=(ObjState& a, ObjState b) noexcept {
    return a = a | b;
}

// ── Alignment ─────────────────────────────────────────────────────────────────
enum class Align : uint8_t {
    Default       = 0,
    TopLeft,  TopMid,  TopRight,
    BottomLeft, BottomMid, BottomRight,
    LeftMid,  Center,  RightMid,
    OutTopLeft, OutTopMid, OutTopRight,
    OutBottomLeft, OutBottomMid, OutBottomRight,
    OutLeftTop, OutLeftMid, OutLeftBottom,
    OutRightTop, OutRightMid, OutRightBottom,
};

// ── Scroll helpers ────────────────────────────────────────────────────────────
enum class ScrollbarMode : uint8_t { Off, On, Active, Auto };
enum class Dir           : uint8_t { None=0, Left=1, Right=2, Top=4, Bottom=8,
                                     Hor=3, Ver=12, All=15 };
enum class AnimEnable    : uint8_t { Off = 0, On = 1 };

// Sentinel size values (mirrors LV_SIZE_CONTENT / LV_PCT)
inline constexpr int32_t SizeContent = 0x7FFFFFF0;
inline constexpr int32_t SizePct(int32_t pct) { return 0x80000000 | pct; }

// ── Non-owning reference handle ───────────────────────────────────────────────
// Becomes null automatically when the referenced object is destroyed.
template<typename T>
class ObjectRef {
public:
    ObjectRef() noexcept = default;

    // Implicitly constructible from a reference to T
    // NOLINTNEXTLINE(google-explicit-constructor)
    ObjectRef(T& obj) noexcept : ptr_(&obj) {}

    [[nodiscard]] bool     valid()    const noexcept { return ptr_ != nullptr; }
    [[nodiscard]] T*       get()            noexcept { return ptr_; }
    [[nodiscard]] const T* get()      const noexcept { return ptr_; }

    T& operator*()  const { return *ptr_; }
    T* operator->() const noexcept { return ptr_; }

    explicit operator bool() const noexcept { return valid(); }

    // Called by Object destructor to invalidate all outstanding refs
    void invalidate() noexcept { ptr_ = nullptr; }

private:
    T* ptr_ = nullptr;
};

// ── Object ────────────────────────────────────────────────────────────────────
class Object {
public:
    // Non-copyable, non-movable: objects have stable addresses once parented.
    Object(const Object&)            = delete;
    Object& operator=(const Object&) = delete;
    Object(Object&&)                 = delete;
    Object& operator=(Object&&)      = delete;
    virtual ~Object();

    // ── Tree ──────────────────────────────────────────────────────────────────
    [[nodiscard]] Object*       parent()       noexcept;
    [[nodiscard]] const Object* parent() const noexcept;
    [[nodiscard]] std::size_t   child_count()  const noexcept;
    [[nodiscard]] Object&       child_at(std::size_t index);
    [[nodiscard]] const Object& child_at(std::size_t index) const;

    // Range-based iteration over direct children
    [[nodiscard]] auto children() noexcept
    -> std::ranges::subrange<std::vector<std::unique_ptr<Object>>::iterator>;
    [[nodiscard]] auto children() const noexcept
    -> std::ranges::subrange<std::vector<std::unique_ptr<Object>>::const_iterator>;

    // Create a child widget of type T, forwarding args to T's constructor.
    // T must be derived from Object and constructible as T(Object*, args...).
    template<std::derived_from<Object> T, typename... Args>
    T& create(Args&&... args) {
        auto child = std::make_unique<T>(this, std::forward<Args>(args)...);
        T& ref = *child;
        children_.push_back(std::move(child));
        ref.on_create();
        return ref;
    }

    void remove_child(Object& child);
    void remove_child(std::size_t index);
    void remove_all_children();

    // ── Flags ─────────────────────────────────────────────────────────────────
    Object& add_flag(ObjFlags f)           noexcept;
    Object& remove_flag(ObjFlags f)        noexcept;
    Object& set_flag(ObjFlags f, bool v)   noexcept;
    [[nodiscard]] bool has_flag(ObjFlags f)     const noexcept;
    [[nodiscard]] bool has_flag_any(ObjFlags f) const noexcept;

    // ── States ────────────────────────────────────────────────────────────────
    Object& add_state(ObjState s)          noexcept;
    Object& remove_state(ObjState s)       noexcept;
    Object& set_state(ObjState s, bool v)  noexcept;
    [[nodiscard]] ObjState state()                const noexcept;
    [[nodiscard]] bool     has_state(ObjState s)  const noexcept;

    // ── Geometry ──────────────────────────────────────────────────────────────
    Object& set_pos(int32_t x, int32_t y)     noexcept;
    Object& set_x(int32_t x)                  noexcept;
    Object& set_y(int32_t y)                  noexcept;
    Object& set_size(int32_t w, int32_t h)    noexcept;
    Object& set_width(int32_t w)              noexcept;
    Object& set_height(int32_t h)             noexcept;
    Object& align(Align a,
                  int32_t x_ofs = 0, int32_t y_ofs = 0) noexcept;
    Object& align_to(const Object& base, Align a,
                     int32_t x_ofs = 0, int32_t y_ofs = 0) noexcept;

    [[nodiscard]] int32_t x()      const noexcept;
    [[nodiscard]] int32_t y()      const noexcept;
    [[nodiscard]] int32_t width()  const noexcept;
    [[nodiscard]] int32_t height() const noexcept;
    [[nodiscard]] Area    bounds() const noexcept;

    // ── Scroll ────────────────────────────────────────────────────────────────
    Object& set_scrollbar_mode(ScrollbarMode m)  noexcept;
    Object& set_scroll_dir(Dir d)                noexcept;
    void    scroll_to(int32_t x, int32_t y, AnimEnable anim) noexcept;
    void    scroll_by(int32_t dx, int32_t dy, AnimEnable anim) noexcept;

    // ── Styles ────────────────────────────────────────────────────────────────
    Object& add_style(const Style& s,
                      StyleSelector sel = {});
    Object& remove_style(const Style& s,
                         StyleSelector sel = {});
    Object& remove_all_styles();

    // ── Events ────────────────────────────────────────────────────────────────
    using Handler = std::move_only_function<void(Event&)>;

    [[nodiscard]] EventHandle on(EventCode code, Handler handler);
    void remove_event(EventHandle& h);
    void send_event(EventCode code, void* param = nullptr);

    // ── Layout ────────────────────────────────────────────────────────────────
    Object& mark_layout_dirty() noexcept;

    // ── Display / Screen accessors ────────────────────────────────────────────
    [[nodiscard]] Display*       display()       noexcept;
    [[nodiscard]] const Display* display() const noexcept;
    [[nodiscard]] Screen*        screen()        noexcept;

    void invalidate()            noexcept;
    void invalidate_area(Area a) noexcept;

protected:
    explicit Object(Object* parent);

    // Extension points for subclasses
    virtual void on_create()                     {}
    virtual void on_delete()                     {}
    virtual void on_event(Event& e)              { (void)e; }
    virtual void on_draw(Layer& ctx)             { (void)ctx; }
    virtual void on_size_changed()               {}
    virtual void on_style_changed()              {}
    virtual void on_child_changed(Object& child) { (void)child; }

private:
    Object*                                  parent_   = nullptr;
    std::vector<std::unique_ptr<Object>>     children_;
    ObjFlags                                 flags_    = ObjFlags::Clickable
                                                       | ObjFlags::Scrollable;
    ObjState                                 state_    = ObjState::Default;

    // Geometry (in parent-relative coords)
    int32_t x_ = 0, y_ = 0, w_ = 0, h_ = 0;

    // Style list — forward declared; defined in style.hpp
    struct StyleEntry;
    std::vector<StyleEntry> styles_;

    // Event handler list — forward declared; defined in event.hpp
    struct EventEntry;
    std::vector<EventEntry> events_;
};

} // namespace lv
