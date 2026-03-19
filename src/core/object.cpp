// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — src/core/object.cpp
// Upstream LVGL baseline: v9.5.0  https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// Implementation of lv::Object — the base class for every widget in the tree.
// Phase 1: object tree, flags, states, geometry (set_pos/size/align), scroll.
// Phase 2: style methods (stubs).
// Phase 3: event methods (stubs).

#include "lvgl/core/object.hpp"

#include <algorithm>
#include <cassert>
#include <memory>
#include <stdexcept>

#include "lvgl/core/event.hpp"   // EventHandle (for remove_event stub)
#include "lvgl/core/screen.hpp"  // Screen (for screen()/display() traversal)

namespace lv {

// ── Private implementation ────────────────────────────────────────────────────

struct Object::Impl {
    ObjFlags      flags         = ObjFlags::Clickable | ObjFlags::Scrollable;
    ObjState      state         = ObjState::Default;
    int32_t       x             = 0;
    int32_t       y             = 0;
    int32_t       w             = 0;
    int32_t       h             = 0;
    ScrollbarMode scrollbar_mode = ScrollbarMode::Auto;
    Dir           scroll_dir    = Dir::All;
    int32_t       scroll_x      = 0;
    int32_t       scroll_y      = 0;
    bool          layout_dirty  = false;

    // Validity token: shared with all ObjectRef<T> instances that point here.
    // Set to false in ~Object() so live refs become invalid atomically.
    std::shared_ptr<bool> validity_token = std::make_shared<bool>(true);
};

// ── Constructor / Destructor ──────────────────────────────────────────────────

Object::Object(Object* parent)
    : parent_(parent), impl_(std::make_unique<Impl>()) {
    if (parent_) {
        // do_add_child is called by create<T>() AFTER construction, so the
        // object is not yet in parent's children_ here.  Nothing to do.
    }
}

Object::~Object() {
    // NOTE: calling on_delete() here would only dispatch to Object::on_delete()
    // because C++ resets the vtable to the base class type during destruction.
    // Derived classes MUST call on_delete() from their own destructor bodies if
    // they need the pre-deletion lifecycle hook.  The framework sends
    // EventCode::Deleted in Phase 3 via the event system before destruction.

    // Invalidate all outstanding ObjectRefs before children are destroyed.
    *impl_->validity_token = false;
    // children_ unique_ptrs are destroyed here (recursive).
}

// ── Private helper ────────────────────────────────────────────────────────────

void Object::do_add_child(std::unique_ptr<Object> child) {
    children_.push_back(std::move(child));
}

// ── Validity token ────────────────────────────────────────────────────────────

std::weak_ptr<bool> Object::validity_token() const noexcept {
    return impl_->validity_token;
}

// ── Tree ──────────────────────────────────────────────────────────────────────

Object* Object::parent() noexcept {
    return parent_;
}
const Object* Object::parent() const noexcept {
    return parent_;
}

std::size_t Object::child_count() const noexcept {
    return children_.size();
}

Object& Object::child_at(std::size_t index) {
    if (index >= children_.size())
        throw std::out_of_range("Object::child_at: index out of range");
    return *children_[index];
}
const Object& Object::child_at(std::size_t index) const {
    if (index >= children_.size())
        throw std::out_of_range("Object::child_at: index out of range");
    return *children_[index];
}

auto Object::children() noexcept
    -> std::ranges::subrange<std::vector<std::unique_ptr<Object>>::iterator> {
    return std::ranges::subrange{children_.begin(), children_.end()};
}
auto Object::children() const noexcept
    -> std::ranges::subrange<
           std::vector<std::unique_ptr<Object>>::const_iterator> {
    return std::ranges::subrange{children_.cbegin(), children_.cend()};
}

void Object::remove_child(Object& child) {
    auto it = std::find_if(children_.begin(), children_.end(),
                           [&](const auto& p) { return p.get() == &child; });
    if (it != children_.end())
        children_.erase(it);
}

void Object::remove_child(std::size_t index) {
    if (index >= children_.size())
        throw std::out_of_range("Object::remove_child: index out of range");
    children_.erase(children_.begin() + static_cast<std::ptrdiff_t>(index));
}

void Object::remove_all_children() {
    children_.clear();
}

// ── Flags ─────────────────────────────────────────────────────────────────────

Object& Object::add_flag(ObjFlags f) noexcept {
    impl_->flags |= f;
    return *this;
}
Object& Object::remove_flag(ObjFlags f) noexcept {
    impl_->flags &= ~f;
    return *this;
}
Object& Object::set_flag(ObjFlags f, bool v) noexcept {
    return v ? add_flag(f) : remove_flag(f);
}
bool Object::has_flag(ObjFlags f) const noexcept {
    // All bits in f must be set
    return (impl_->flags & f) == f;
}
bool Object::has_flag_any(ObjFlags f) const noexcept {
    return (impl_->flags & f) != ObjFlags::None;
}

// ── States ────────────────────────────────────────────────────────────────────

Object& Object::add_state(ObjState s) noexcept {
    impl_->state |= s;
    return *this;
}
Object& Object::remove_state(ObjState s) noexcept {
    impl_->state &= ~s;
    return *this;
}
Object& Object::set_state(ObjState s, bool v) noexcept {
    return v ? add_state(s) : remove_state(s);
}
ObjState Object::state() const noexcept {
    return impl_->state;
}
bool Object::has_state(ObjState s) const noexcept {
    return (impl_->state & s) == s;
}

// ── Geometry ──────────────────────────────────────────────────────────────────

Object& Object::set_pos(int32_t x, int32_t y) noexcept {
    impl_->x = x;
    impl_->y = y;
    return *this;
}
Object& Object::set_x(int32_t x) noexcept {
    impl_->x = x;
    return *this;
}
Object& Object::set_y(int32_t y) noexcept {
    impl_->y = y;
    return *this;
}
Object& Object::set_size(int32_t w, int32_t h) noexcept {
    impl_->w = w;
    impl_->h = h;
    on_size_changed();
    return *this;
}
Object& Object::set_width(int32_t w) noexcept {
    impl_->w = w;
    on_size_changed();
    return *this;
}
Object& Object::set_height(int32_t h) noexcept {
    impl_->h = h;
    on_size_changed();
    return *this;
}

int32_t Object::x()      const noexcept { return impl_->x; }
int32_t Object::y()      const noexcept { return impl_->y; }
int32_t Object::width()  const noexcept { return impl_->w; }
int32_t Object::height() const noexcept { return impl_->h; }
Area    Object::bounds() const noexcept {
    return Area::from_size(impl_->x, impl_->y, impl_->w, impl_->h);
}

// ── align() ──────────────────────────────────────────────────────────────────
//
// Positions this object within its parent using standard LVGL alignment rules
// (ref: lv_obj_align in LVGL v9.5.0 lv_obj_pos.c).
// "In" alignments place the object inside the parent.
// "Out" alignments place it outside the parent boundary.

Object& Object::align(Align a, int32_t x_ofs, int32_t y_ofs) noexcept {
    if (a == Align::Default) return *this;
    int32_t pw = parent_ ? parent_->width()  : 0;
    int32_t ph = parent_ ? parent_->height() : 0;
    int32_t w  = impl_->w;
    int32_t h  = impl_->h;
    int32_t nx = 0;
    int32_t ny = 0;

    switch (a) {
        // ── Inside parent ────────────────────────────────────────────────────
        case Align::TopLeft:        nx = 0;          ny = 0;         break;
        case Align::TopMid:         nx = (pw - w)/2; ny = 0;         break;
        case Align::TopRight:       nx = pw - w;     ny = 0;         break;
        case Align::BottomLeft:     nx = 0;          ny = ph - h;    break;
        case Align::BottomMid:      nx = (pw - w)/2; ny = ph - h;    break;
        case Align::BottomRight:    nx = pw - w;     ny = ph - h;    break;
        case Align::LeftMid:        nx = 0;          ny = (ph - h)/2; break;
        case Align::Center:         nx = (pw - w)/2; ny = (ph - h)/2; break;
        case Align::RightMid:       nx = pw - w;     ny = (ph - h)/2; break;
        // ── Outside parent ───────────────────────────────────────────────────
        case Align::OutTopLeft:     nx = 0;          ny = -h;        break;
        case Align::OutTopMid:      nx = (pw - w)/2; ny = -h;        break;
        case Align::OutTopRight:    nx = pw - w;     ny = -h;        break;
        case Align::OutBottomLeft:  nx = 0;          ny = ph;        break;
        case Align::OutBottomMid:   nx = (pw - w)/2; ny = ph;        break;
        case Align::OutBottomRight: nx = pw - w;     ny = ph;        break;
        case Align::OutLeftTop:     nx = -w;         ny = 0;         break;
        case Align::OutLeftMid:     nx = -w;         ny = (ph - h)/2; break;
        case Align::OutLeftBottom:  nx = -w;         ny = ph - h;    break;
        case Align::OutRightTop:    nx = pw;         ny = 0;         break;
        case Align::OutRightMid:    nx = pw;         ny = (ph - h)/2; break;
        case Align::OutRightBottom: nx = pw;         ny = ph - h;    break;
        default: break;
    }
    impl_->x = nx + x_ofs;
    impl_->y = ny + y_ofs;
    return *this;
}

// ── align_to() ────────────────────────────────────────────────────────────────
//
// Positions this object relative to @p base (which need not be the parent).
// Coordinates are computed in the same coordinate space as this object's
// parent (ref: lv_obj_align_to in LVGL v9.5.0 lv_obj_pos.c).

Object& Object::align_to(const Object& base,
                          Align         a,
                          int32_t       x_ofs,
                          int32_t       y_ofs) noexcept {
    if (a == Align::Default) return *this;
    int32_t bx = base.x();
    int32_t by = base.y();
    int32_t bw = base.width();
    int32_t bh = base.height();
    int32_t w  = impl_->w;
    int32_t h  = impl_->h;
    int32_t nx = 0;
    int32_t ny = 0;

    switch (a) {
        // ── Inside base ──────────────────────────────────────────────────────
        case Align::TopLeft:        nx = bx;              ny = by;           break;
        case Align::TopMid:         nx = bx+(bw-w)/2;     ny = by;           break;
        case Align::TopRight:       nx = bx + bw - w;     ny = by;           break;
        case Align::BottomLeft:     nx = bx;              ny = by + bh - h;  break;
        case Align::BottomMid:      nx = bx+(bw-w)/2;     ny = by + bh - h;  break;
        case Align::BottomRight:    nx = bx + bw - w;     ny = by + bh - h;  break;
        case Align::LeftMid:        nx = bx;              ny = by+(bh-h)/2;  break;
        case Align::Center:         nx = bx+(bw-w)/2;     ny = by+(bh-h)/2;  break;
        case Align::RightMid:       nx = bx + bw - w;     ny = by+(bh-h)/2;  break;
        // ── Outside base ─────────────────────────────────────────────────────
        case Align::OutTopLeft:     nx = bx;              ny = by - h;       break;
        case Align::OutTopMid:      nx = bx+(bw-w)/2;     ny = by - h;       break;
        case Align::OutTopRight:    nx = bx + bw - w;     ny = by - h;       break;
        case Align::OutBottomLeft:  nx = bx;              ny = by + bh;      break;
        case Align::OutBottomMid:   nx = bx+(bw-w)/2;     ny = by + bh;      break;
        case Align::OutBottomRight: nx = bx + bw - w;     ny = by + bh;      break;
        case Align::OutLeftTop:     nx = bx - w;          ny = by;           break;
        case Align::OutLeftMid:     nx = bx - w;          ny = by+(bh-h)/2;  break;
        case Align::OutLeftBottom:  nx = bx - w;          ny = by + bh - h;  break;
        case Align::OutRightTop:    nx = bx + bw;         ny = by;           break;
        case Align::OutRightMid:    nx = bx + bw;         ny = by+(bh-h)/2;  break;
        case Align::OutRightBottom: nx = bx + bw;         ny = by + bh - h;  break;
        default: break;
    }
    impl_->x = nx + x_ofs;
    impl_->y = ny + y_ofs;
    return *this;
}

// ── Scroll ────────────────────────────────────────────────────────────────────

Object& Object::set_scrollbar_mode(ScrollbarMode m) noexcept {
    impl_->scrollbar_mode = m;
    return *this;
}
Object& Object::set_scroll_dir(Dir d) noexcept {
    impl_->scroll_dir = d;
    return *this;
}
ScrollbarMode Object::scrollbar_mode() const noexcept {
    return impl_->scrollbar_mode;
}
Dir Object::scroll_dir() const noexcept {
    return impl_->scroll_dir;
}
void Object::scroll_to(int32_t x, int32_t y, AnimEnable /*anim*/) noexcept {
    impl_->scroll_x = x;
    impl_->scroll_y = y;
}
void Object::scroll_by(int32_t dx, int32_t dy, AnimEnable /*anim*/) noexcept {
    impl_->scroll_x += dx;
    impl_->scroll_y += dy;
}
int32_t Object::scroll_x() const noexcept { return impl_->scroll_x; }
int32_t Object::scroll_y() const noexcept { return impl_->scroll_y; }

// ── Styles (Phase 2 stubs) ────────────────────────────────────────────────────

Object& Object::add_style(const Style& /*s*/, StyleSelector /*sel*/) {
    return *this;
}
Object& Object::remove_style(const Style& /*s*/, StyleSelector /*sel*/) {
    return *this;
}
Object& Object::remove_all_styles() {
    return *this;
}

// ── Events (Phase 3 stubs) ────────────────────────────────────────────────────

EventHandle Object::on(EventCode /*code*/, Handler /*handler*/) {
    return EventHandle{};
}
void Object::remove_event(EventHandle& /*h*/) {}
void Object::send_event(EventCode code, void* param) {
    // Phase 1: call on_event directly on this object only (no bubbling/trickle).
    Event e{*this, code, param};
    on_event(e);
}

// ── Layout ────────────────────────────────────────────────────────────────────

Object& Object::mark_layout_dirty() noexcept {
    impl_->layout_dirty = true;
    return *this;
}

// ── Display / Screen ──────────────────────────────────────────────────────────

Display* Object::display() noexcept {
    return screen() ? screen()->owner_display() : nullptr;
}
const Display* Object::display() const noexcept {
    return const_cast<Object*>(this)->display();
}

Screen* Object::screen() noexcept {
    Object* cur = this;
    while (cur->parent_ != nullptr)
        cur = cur->parent_;
    return dynamic_cast<Screen*>(cur);
}

void Object::invalidate() noexcept {}
void Object::invalidate_area(Area /*a*/) noexcept {}

}  // namespace lv
