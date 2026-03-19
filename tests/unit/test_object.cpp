// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// tests/unit/test_object.cpp
// Phase 1 unit tests for lv::Object, lv::Screen, and lv::ObjectRef.
//
// Coverage targets (Phase 1 checklist from docs/design/11-implementation-plan.md):
//  ✓ Object::create<T>() constructs and parents children
//  ✓ Parent destructor recursively destroys children
//  ✓ ObjectRef<T> nullifies on child destroy
//  ✓ Flags and states are bitwise correct
//  ✓ set_pos, set_size, align write to Impl fields
//  ✓ children() returns a correct forward range
//  ✓ All Phase-1 unit tests pass

#include <gtest/gtest.h>

#include <stdexcept>
#include <vector>

#include "lvgl/core/event.hpp"
#include "lvgl/core/object.hpp"
#include "lvgl/core/screen.hpp"

using namespace lv;

// ── Test fixture helpers ──────────────────────────────────────────────────────

// Minimal concrete Object subclass for testing
struct Widget : public Object {
    explicit Widget(Object* parent) : Object(parent) {}

    bool on_create_called = false;
    bool on_delete_called = false;
    bool on_size_changed_called = false;

    void on_create() override { on_create_called = true; }
    void on_delete() override { on_delete_called = true; }
    void on_size_changed() override { on_size_changed_called = true; }
};

// ── Screen construction ───────────────────────────────────────────────────────

TEST(Screen, DefaultConstruction) {
    Screen s;
    EXPECT_EQ(s.parent(), nullptr);
    EXPECT_EQ(s.child_count(), 0u);
    EXPECT_EQ(s.owner_display(), nullptr);
}

TEST(Screen, HasNoParent) {
    Screen s;
    EXPECT_EQ(s.parent(), nullptr);
}

TEST(Screen, OwnerDisplayNullByDefault) {
    Screen s;
    EXPECT_EQ(s.owner_display(), nullptr);
}

TEST(Screen, ScreenMethodReturnsItself) {
    Screen s;
    EXPECT_EQ(s.screen(), &s);
}

TEST(Screen, DisplayReturnsNullWhenNoDisplay) {
    Screen s;
    EXPECT_EQ(s.display(), nullptr);
}

// ── Object::create<T>() ───────────────────────────────────────────────────────

TEST(ObjectTree, CreateChildSetsParent) {
    Screen root;
    auto& child = root.create<Widget>();
    EXPECT_EQ(child.parent(), &root);
}

TEST(ObjectTree, CreateChildIncreasesChildCount) {
    Screen root;
    EXPECT_EQ(root.child_count(), 0u);
    root.create<Widget>();
    EXPECT_EQ(root.child_count(), 1u);
    root.create<Widget>();
    EXPECT_EQ(root.child_count(), 2u);
}

TEST(ObjectTree, CreateCallsOnCreate) {
    Screen root;
    auto& w = root.create<Widget>();
    EXPECT_TRUE(w.on_create_called);
}

TEST(ObjectTree, CreateGrandchild) {
    Screen root;
    auto& child = root.create<Widget>();
    auto& grand = child.create<Widget>();
    EXPECT_EQ(grand.parent(), &child);
    EXPECT_EQ(child.child_count(), 1u);
}

TEST(ObjectTree, CreateReturnsRef) {
    Screen root;
    Widget& ref = root.create<Widget>();
    EXPECT_EQ(root.child_count(), 1u);
    EXPECT_EQ(&root.child_at(0), &ref);
}

// ── child_at ─────────────────────────────────────────────────────────────────

TEST(ObjectTree, ChildAtValid) {
    Screen root;
    auto& c0 = root.create<Widget>();
    auto& c1 = root.create<Widget>();
    EXPECT_EQ(&root.child_at(0), &c0);
    EXPECT_EQ(&root.child_at(1), &c1);
}

TEST(ObjectTree, ChildAtConstValid) {
    Screen root;
    root.create<Widget>();
    const Object& croot = root;
    EXPECT_EQ(croot.child_count(), 1u);
    (void)croot.child_at(0);  // should not throw
}

TEST(ObjectTree, ChildAtOutOfRangeThrows) {
    Screen root;
    // Cast to void so [[nodiscard]] warning is suppressed inside EXPECT_THROW
    EXPECT_THROW((void)root.child_at(0), std::out_of_range);
}

TEST(ObjectTree, ChildAtOutOfRangeConstThrows) {
    Screen root;
    const Object& croot = root;
    EXPECT_THROW((void)croot.child_at(0), std::out_of_range);
}

// ── children() range ─────────────────────────────────────────────────────────

TEST(ObjectTree, ChildrenEmptyRange) {
    Screen root;
    int count = 0;
    for ([[maybe_unused]] auto& c : root.children()) ++count;
    EXPECT_EQ(count, 0);
}

TEST(ObjectTree, ChildrenRangeIteratesAll) {
    Screen root;
    root.create<Widget>();
    root.create<Widget>();
    root.create<Widget>();
    int count = 0;
    for ([[maybe_unused]] auto& c : root.children()) ++count;
    EXPECT_EQ(count, 3);
}

TEST(ObjectTree, ChildrenConstRange) {
    Screen root;
    root.create<Widget>();
    const Object& croot = root;
    int count = 0;
    for ([[maybe_unused]] auto& c : croot.children()) ++count;
    EXPECT_EQ(count, 1);
}

TEST(ObjectTree, ChildrenRangeDeref) {
    Screen root;
    auto& w = root.create<Widget>();
    for (auto& ptr : root.children()) {
        EXPECT_EQ(ptr.get(), &w);
    }
}

// ── remove_child ─────────────────────────────────────────────────────────────

TEST(ObjectTree, RemoveChildByRef) {
    Screen root;
    auto& c0 = root.create<Widget>();
    auto& c1 = root.create<Widget>();
    root.remove_child(c0);
    EXPECT_EQ(root.child_count(), 1u);
    EXPECT_EQ(&root.child_at(0), &c1);
}

TEST(ObjectTree, RemoveChildByRefNotFound) {
    Screen root;
    Widget other{nullptr};
    // Removing a non-child should be a no-op
    root.remove_child(other);
    EXPECT_EQ(root.child_count(), 0u);
}

TEST(ObjectTree, RemoveChildByIndex) {
    Screen root;
    root.create<Widget>();
    auto& c1 = root.create<Widget>();
    root.remove_child(static_cast<std::size_t>(0));
    EXPECT_EQ(root.child_count(), 1u);
    EXPECT_EQ(&root.child_at(0), &c1);
}

TEST(ObjectTree, RemoveChildByIndexOutOfRangeThrows) {
    Screen root;
    EXPECT_THROW(root.remove_child(static_cast<std::size_t>(0)), std::out_of_range);
}
TEST(ObjectTree, RemoveAllChildren) {
    Screen root;
    root.create<Widget>();
    root.create<Widget>();
    root.create<Widget>();
    root.remove_all_children();
    EXPECT_EQ(root.child_count(), 0u);
}

// ── Destructor / recursive destruction ───────────────────────────────────────

TEST(ObjectTree, DestructorCallsOnDelete) {
    bool deleted = false;
    struct Tracked : public Object {
        bool& flag_;
        explicit Tracked(Object* p, bool& f) : Object(p), flag_(f) {}
        // Use the C++ destructor (not on_delete()) for reliable teardown:
        // virtual dispatch during ~Object() uses Object's vtable, not Tracked's.
        ~Tracked() override { flag_ = true; }
    };
    {
        Screen root;
        root.create<Tracked>(std::ref(deleted));
    }
    EXPECT_TRUE(deleted);
}

TEST(ObjectTree, ParentDestructorDestroysChildren) {
    int delete_count = 0;
    struct Counter : public Object {
        int& count_;
        explicit Counter(Object* p, int& c) : Object(p), count_(c) {}
        ~Counter() override { ++count_; }
    };
    {
        Screen root;
        root.create<Counter>(std::ref(delete_count));
        root.create<Counter>(std::ref(delete_count));
    }
    EXPECT_EQ(delete_count, 2);
}

TEST(ObjectTree, RecursiveDestructionDeepTree) {
    int count = 0;
    struct Counter : public Object {
        int& c_;
        explicit Counter(Object* p, int& c) : Object(p), c_(c) {}
        ~Counter() override { ++c_; }
    };
    {
        Screen root;
        auto& c1 = root.create<Counter>(std::ref(count));
        auto& c2 = c1.create<Counter>(std::ref(count));
        c2.create<Counter>(std::ref(count));  // depth 3
    }
    EXPECT_EQ(count, 3);
}

// ── ObjectRef validity ────────────────────────────────────────────────────────

TEST(ObjectRef, ValidAfterConstruction) {
    Screen root;
    auto& child = root.create<Widget>();
    ObjectRef<Widget> ref{child};
    EXPECT_TRUE(ref.valid());
    EXPECT_TRUE(static_cast<bool>(ref));
    EXPECT_EQ(ref.get(), &child);
}

TEST(ObjectRef, DefaultConstructedIsInvalid) {
    ObjectRef<Widget> ref;
    EXPECT_FALSE(ref.valid());
    EXPECT_EQ(ref.get(), nullptr);
}

TEST(ObjectRef, InvalidatedAfterObjectDestroyed) {
    ObjectRef<Widget> ref;
    {
        Screen root;
        auto& child = root.create<Widget>();
        ref = ObjectRef<Widget>{child};
        EXPECT_TRUE(ref.valid());
    }  // root (and child) destroyed here
    EXPECT_FALSE(ref.valid());
    EXPECT_EQ(ref.get(), nullptr);
}

TEST(ObjectRef, InvalidatedAfterRemoveChild) {
    Screen root;
    auto& child = root.create<Widget>();
    ObjectRef<Widget> ref{child};
    EXPECT_TRUE(ref.valid());
    root.remove_child(child);  // destroys child via unique_ptr
    EXPECT_FALSE(ref.valid());
}

TEST(ObjectRef, DerefOperatorReturnsObject) {
    Screen root;
    auto& child = root.create<Widget>();
    ObjectRef<Widget> ref{child};
    EXPECT_EQ(&(*ref), &child);
    EXPECT_EQ(ref->parent(), &root);
}

TEST(ObjectRef, ImplicitConstructionFromRef) {
    Screen root;
    auto& child = root.create<Widget>();
    ObjectRef<Widget> ref = child;  // implicit
    EXPECT_TRUE(ref.valid());
}

// ── Flags ─────────────────────────────────────────────────────────────────────

TEST(ObjectFlags, DefaultIncludesClickableAndScrollable) {
    Screen root;
    EXPECT_TRUE(root.has_flag(ObjFlags::Clickable));
    EXPECT_TRUE(root.has_flag(ObjFlags::Scrollable));
}

TEST(ObjectFlags, AddFlag) {
    Screen root;
    root.add_flag(ObjFlags::Hidden);
    EXPECT_TRUE(root.has_flag(ObjFlags::Hidden));
}

TEST(ObjectFlags, RemoveFlag) {
    Screen root;
    root.add_flag(ObjFlags::Hidden);
    root.remove_flag(ObjFlags::Hidden);
    EXPECT_FALSE(root.has_flag(ObjFlags::Hidden));
}

TEST(ObjectFlags, SetFlagTrue) {
    Screen root;
    root.set_flag(ObjFlags::Hidden, true);
    EXPECT_TRUE(root.has_flag(ObjFlags::Hidden));
}

TEST(ObjectFlags, SetFlagFalse) {
    Screen root;
    root.add_flag(ObjFlags::Hidden);
    root.set_flag(ObjFlags::Hidden, false);
    EXPECT_FALSE(root.has_flag(ObjFlags::Hidden));
}

TEST(ObjectFlags, HasFlagRequiresAllBits) {
    Screen root;
    root.add_flag(ObjFlags::Hidden);
    // has_flag checks ALL bits — {Hidden|Clickable} fails if only Hidden is set
    EXPECT_FALSE(root.has_flag(ObjFlags::Hidden | ObjFlags::EventBubble));
}

TEST(ObjectFlags, HasFlagAnyRequiresAtLeastOneBit) {
    Screen root;
    root.add_flag(ObjFlags::Hidden);
    EXPECT_TRUE(root.has_flag_any(ObjFlags::Hidden | ObjFlags::EventBubble));
    EXPECT_FALSE(root.has_flag_any(ObjFlags::EventBubble));
}

TEST(ObjectFlags, ChainedFlagOperations) {
    Screen root;
    root.add_flag(ObjFlags::Hidden)
        .add_flag(ObjFlags::EventBubble)
        .remove_flag(ObjFlags::Hidden);
    EXPECT_FALSE(root.has_flag(ObjFlags::Hidden));
    EXPECT_TRUE(root.has_flag(ObjFlags::EventBubble));
}

// ── States ────────────────────────────────────────────────────────────────────

TEST(ObjectState, DefaultStateIsDefault) {
    Screen root;
    EXPECT_EQ(root.state(), ObjState::Default);
    EXPECT_FALSE(root.has_state(ObjState::Pressed));
}

TEST(ObjectState, AddState) {
    Screen root;
    root.add_state(ObjState::Pressed);
    EXPECT_TRUE(root.has_state(ObjState::Pressed));
}

TEST(ObjectState, RemoveState) {
    Screen root;
    root.add_state(ObjState::Pressed);
    root.remove_state(ObjState::Pressed);
    EXPECT_EQ(root.state(), ObjState::Default);
}

TEST(ObjectState, SetStateTrue) {
    Screen root;
    root.set_state(ObjState::Focused, true);
    EXPECT_TRUE(root.has_state(ObjState::Focused));
}

TEST(ObjectState, SetStateFalse) {
    Screen root;
    root.add_state(ObjState::Focused);
    root.set_state(ObjState::Focused, false);
    EXPECT_FALSE(root.has_state(ObjState::Focused));
}

TEST(ObjectState, MultipleStates) {
    Screen root;
    root.add_state(ObjState::Pressed).add_state(ObjState::Focused);
    EXPECT_TRUE(root.has_state(ObjState::Pressed));
    EXPECT_TRUE(root.has_state(ObjState::Focused));
    EXPECT_EQ(root.state(), ObjState::Pressed | ObjState::Focused);
}

TEST(ObjectState, RemoveOneOfMultiple) {
    Screen root;
    root.add_state(ObjState::Pressed | ObjState::Focused);
    root.remove_state(ObjState::Pressed);
    EXPECT_FALSE(root.has_state(ObjState::Pressed));
    EXPECT_TRUE(root.has_state(ObjState::Focused));
}

TEST(ObjectState, HasStateChecksAllBits) {
    Screen root;
    root.add_state(ObjState::Pressed);
    EXPECT_FALSE(root.has_state(ObjState::Pressed | ObjState::Focused));
}

// ── Geometry — set/get ────────────────────────────────────────────────────────

TEST(ObjectGeometry, DefaultGeometryIsZero) {
    Screen root;
    EXPECT_EQ(root.x(), 0);
    EXPECT_EQ(root.y(), 0);
    EXPECT_EQ(root.width(),  0);
    EXPECT_EQ(root.height(), 0);
}

TEST(ObjectGeometry, SetPos) {
    Screen root;
    root.set_pos(10, 20);
    EXPECT_EQ(root.x(), 10);
    EXPECT_EQ(root.y(), 20);
}

TEST(ObjectGeometry, SetX) {
    Screen root;
    root.set_x(42);
    EXPECT_EQ(root.x(), 42);
    EXPECT_EQ(root.y(),  0);
}

TEST(ObjectGeometry, SetY) {
    Screen root;
    root.set_y(99);
    EXPECT_EQ(root.x(),  0);
    EXPECT_EQ(root.y(), 99);
}

TEST(ObjectGeometry, SetSize) {
    Screen root;
    root.set_size(320, 240);
    EXPECT_EQ(root.width(),  320);
    EXPECT_EQ(root.height(), 240);
}

TEST(ObjectGeometry, SetSizeCallsOnSizeChanged) {
    Screen root;
    auto& w = root.create<Widget>();
    w.set_size(100, 50);
    EXPECT_TRUE(w.on_size_changed_called);
}

TEST(ObjectGeometry, SetWidth) {
    Screen root;
    root.set_width(160);
    EXPECT_EQ(root.width(), 160);
}

TEST(ObjectGeometry, SetHeight) {
    Screen root;
    root.set_height(80);
    EXPECT_EQ(root.height(), 80);
}

TEST(ObjectGeometry, Bounds) {
    Screen root;
    root.set_pos(5, 10).set_size(100, 50);
    auto b = root.bounds();
    EXPECT_EQ(b.x1, 5);
    EXPECT_EQ(b.y1, 10);
    EXPECT_EQ(b.x2, 104);  // x + w - 1
    EXPECT_EQ(b.y2, 59);   // y + h - 1
}

TEST(ObjectGeometry, NegativePos) {
    Screen root;
    root.set_pos(-10, -20);
    EXPECT_EQ(root.x(), -10);
    EXPECT_EQ(root.y(), -20);
}

TEST(ObjectGeometry, Chaining) {
    Screen root;
    root.set_pos(1, 2).set_size(3, 4);
    EXPECT_EQ(root.x(), 1);
    EXPECT_EQ(root.y(), 2);
    EXPECT_EQ(root.width(),  3);
    EXPECT_EQ(root.height(), 4);
}

// ── align() ──────────────────────────────────────────────────────────────────

// Helper — builds a Screen root of given size, creates a Widget child of given
// size, aligns the child, and returns the child's (x, y).
static std::pair<int32_t, int32_t> aligned_pos(
    int32_t parent_w, int32_t parent_h,
    int32_t child_w,  int32_t child_h,
    Align   a,
    int32_t xofs = 0, int32_t yofs = 0)
{
    Screen root;
    root.set_size(parent_w, parent_h);
    auto& c = root.create<Widget>();
    c.set_size(child_w, child_h);
    c.align(a, xofs, yofs);
    return {c.x(), c.y()};
}

TEST(ObjectAlign, DefaultDoesNothing) {
    Screen root;
    root.set_size(200, 100);
    auto& c = root.create<Widget>();
    c.set_pos(10, 20).set_size(40, 20);
    c.align(Align::Default);
    EXPECT_EQ(c.x(), 10);
    EXPECT_EQ(c.y(), 20);
}

TEST(ObjectAlign, TopLeft) {
    auto [x, y] = aligned_pos(200, 100, 40, 20, Align::TopLeft);
    EXPECT_EQ(x, 0);
    EXPECT_EQ(y, 0);
}

TEST(ObjectAlign, TopMid) {
    auto [x, y] = aligned_pos(200, 100, 40, 20, Align::TopMid);
    EXPECT_EQ(x, (200 - 40) / 2);
    EXPECT_EQ(y, 0);
}

TEST(ObjectAlign, TopRight) {
    auto [x, y] = aligned_pos(200, 100, 40, 20, Align::TopRight);
    EXPECT_EQ(x, 200 - 40);
    EXPECT_EQ(y, 0);
}

TEST(ObjectAlign, BottomLeft) {
    auto [x, y] = aligned_pos(200, 100, 40, 20, Align::BottomLeft);
    EXPECT_EQ(x, 0);
    EXPECT_EQ(y, 100 - 20);
}

TEST(ObjectAlign, BottomMid) {
    auto [x, y] = aligned_pos(200, 100, 40, 20, Align::BottomMid);
    EXPECT_EQ(x, (200 - 40) / 2);
    EXPECT_EQ(y, 100 - 20);
}

TEST(ObjectAlign, BottomRight) {
    auto [x, y] = aligned_pos(200, 100, 40, 20, Align::BottomRight);
    EXPECT_EQ(x, 200 - 40);
    EXPECT_EQ(y, 100 - 20);
}

TEST(ObjectAlign, LeftMid) {
    auto [x, y] = aligned_pos(200, 100, 40, 20, Align::LeftMid);
    EXPECT_EQ(x, 0);
    EXPECT_EQ(y, (100 - 20) / 2);
}

TEST(ObjectAlign, Center) {
    auto [x, y] = aligned_pos(200, 100, 40, 20, Align::Center);
    EXPECT_EQ(x, (200 - 40) / 2);
    EXPECT_EQ(y, (100 - 20) / 2);
}

TEST(ObjectAlign, RightMid) {
    auto [x, y] = aligned_pos(200, 100, 40, 20, Align::RightMid);
    EXPECT_EQ(x, 200 - 40);
    EXPECT_EQ(y, (100 - 20) / 2);
}

TEST(ObjectAlign, OutTopLeft) {
    auto [x, y] = aligned_pos(200, 100, 40, 20, Align::OutTopLeft);
    EXPECT_EQ(x, 0);
    EXPECT_EQ(y, -20);
}

TEST(ObjectAlign, OutTopMid) {
    auto [x, y] = aligned_pos(200, 100, 40, 20, Align::OutTopMid);
    EXPECT_EQ(x, (200 - 40) / 2);
    EXPECT_EQ(y, -20);
}

TEST(ObjectAlign, OutTopRight) {
    auto [x, y] = aligned_pos(200, 100, 40, 20, Align::OutTopRight);
    EXPECT_EQ(x, 200 - 40);
    EXPECT_EQ(y, -20);
}

TEST(ObjectAlign, OutBottomLeft) {
    auto [x, y] = aligned_pos(200, 100, 40, 20, Align::OutBottomLeft);
    EXPECT_EQ(x, 0);
    EXPECT_EQ(y, 100);
}

TEST(ObjectAlign, OutBottomMid) {
    auto [x, y] = aligned_pos(200, 100, 40, 20, Align::OutBottomMid);
    EXPECT_EQ(x, (200 - 40) / 2);
    EXPECT_EQ(y, 100);
}

TEST(ObjectAlign, OutBottomRight) {
    auto [x, y] = aligned_pos(200, 100, 40, 20, Align::OutBottomRight);
    EXPECT_EQ(x, 200 - 40);
    EXPECT_EQ(y, 100);
}

TEST(ObjectAlign, OutLeftTop) {
    auto [x, y] = aligned_pos(200, 100, 40, 20, Align::OutLeftTop);
    EXPECT_EQ(x, -40);
    EXPECT_EQ(y, 0);
}

TEST(ObjectAlign, OutLeftMid) {
    auto [x, y] = aligned_pos(200, 100, 40, 20, Align::OutLeftMid);
    EXPECT_EQ(x, -40);
    EXPECT_EQ(y, (100 - 20) / 2);
}

TEST(ObjectAlign, OutLeftBottom) {
    auto [x, y] = aligned_pos(200, 100, 40, 20, Align::OutLeftBottom);
    EXPECT_EQ(x, -40);
    EXPECT_EQ(y, 100 - 20);
}

TEST(ObjectAlign, OutRightTop) {
    auto [x, y] = aligned_pos(200, 100, 40, 20, Align::OutRightTop);
    EXPECT_EQ(x, 200);
    EXPECT_EQ(y, 0);
}

TEST(ObjectAlign, OutRightMid) {
    auto [x, y] = aligned_pos(200, 100, 40, 20, Align::OutRightMid);
    EXPECT_EQ(x, 200);
    EXPECT_EQ(y, (100 - 20) / 2);
}

TEST(ObjectAlign, OutRightBottom) {
    auto [x, y] = aligned_pos(200, 100, 40, 20, Align::OutRightBottom);
    EXPECT_EQ(x, 200);
    EXPECT_EQ(y, 100 - 20);
}

TEST(ObjectAlign, CenterWithOffset) {
    auto [x, y] = aligned_pos(200, 100, 40, 20, Align::Center, 5, -3);
    EXPECT_EQ(x, (200 - 40) / 2 + 5);
    EXPECT_EQ(y, (100 - 20) / 2 - 3);
}

// ── align_to() ────────────────────────────────────────────────────────────────

TEST(ObjectAlignTo, DefaultDoesNothing) {
    Screen root;
    root.set_size(300, 200);
    auto& base   = root.create<Widget>();
    auto& target = root.create<Widget>();
    base.set_pos(50, 60).set_size(80, 40);
    target.set_pos(10, 10).set_size(20, 20);
    target.align_to(base, Align::Default);
    EXPECT_EQ(target.x(), 10);
    EXPECT_EQ(target.y(), 10);
}

TEST(ObjectAlignTo, Center) {
    Screen root;
    root.set_size(300, 200);
    auto& base   = root.create<Widget>();
    auto& target = root.create<Widget>();
    base.set_pos(50, 60).set_size(80, 40);
    target.set_size(20, 10);
    target.align_to(base, Align::Center);
    EXPECT_EQ(target.x(), 50 + (80 - 20) / 2);  // 80
    EXPECT_EQ(target.y(), 60 + (40 - 10) / 2);  // 75
}

TEST(ObjectAlignTo, OutTopLeft) {
    Screen root;
    root.set_size(300, 200);
    auto& base   = root.create<Widget>();
    auto& target = root.create<Widget>();
    base.set_pos(100, 80).set_size(60, 30);
    target.set_size(20, 10);
    target.align_to(base, Align::OutTopLeft);
    EXPECT_EQ(target.x(), 100);
    EXPECT_EQ(target.y(), 80 - 10);
}

TEST(ObjectAlignTo, OutRightTop) {
    Screen root;
    root.set_size(300, 200);
    auto& base   = root.create<Widget>();
    auto& target = root.create<Widget>();
    base.set_pos(100, 80).set_size(60, 30);
    target.set_size(20, 10);
    target.align_to(base, Align::OutRightTop);
    EXPECT_EQ(target.x(), 100 + 60);
    EXPECT_EQ(target.y(), 80);
}

TEST(ObjectAlignTo, WithOffset) {
    Screen root;
    root.set_size(300, 200);
    auto& base   = root.create<Widget>();
    auto& target = root.create<Widget>();
    base.set_pos(50, 50).set_size(100, 100);
    target.set_size(40, 40);
    target.align_to(base, Align::Center, 10, -5);
    EXPECT_EQ(target.x(), 50 + (100 - 40) / 2 + 10);
    EXPECT_EQ(target.y(), 50 + (100 - 40) / 2 - 5);
}

// ── Scroll ────────────────────────────────────────────────────────────────────

TEST(ObjectScroll, DefaultScrollbarModeIsAuto) {
    Screen root;
    EXPECT_EQ(root.scrollbar_mode(), ScrollbarMode::Auto);
}

TEST(ObjectScroll, DefaultScrollDirIsAll) {
    Screen root;
    EXPECT_EQ(root.scroll_dir(), Dir::All);
}

TEST(ObjectScroll, SetScrollbarMode) {
    Screen root;
    root.set_scrollbar_mode(ScrollbarMode::Off);
    EXPECT_EQ(root.scrollbar_mode(), ScrollbarMode::Off);
    root.set_scrollbar_mode(ScrollbarMode::On);
    EXPECT_EQ(root.scrollbar_mode(), ScrollbarMode::On);
}

TEST(ObjectScroll, SetScrollDir) {
    Screen root;
    root.set_scroll_dir(Dir::Hor);
    EXPECT_EQ(root.scroll_dir(), Dir::Hor);
}

TEST(ObjectScroll, ScrollToSetsPosition) {
    Screen root;
    root.scroll_to(30, 40, AnimEnable::Off);
    EXPECT_EQ(root.scroll_x(), 30);
    EXPECT_EQ(root.scroll_y(), 40);
}

TEST(ObjectScroll, ScrollByOffsets) {
    Screen root;
    root.scroll_to(10, 20, AnimEnable::Off);
    root.scroll_by(5, -5, AnimEnable::Off);
    EXPECT_EQ(root.scroll_x(), 15);
    EXPECT_EQ(root.scroll_y(), 15);
}

TEST(ObjectScroll, DefaultScrollPositionIsZero) {
    Screen root;
    EXPECT_EQ(root.scroll_x(), 0);
    EXPECT_EQ(root.scroll_y(), 0);
}

// ── Layout dirty ─────────────────────────────────────────────────────────────

TEST(ObjectLayout, MarkLayoutDirtyChains) {
    Screen root;
    // Just verify chaining works — no observable state yet (Phase 4 adds rendering)
    root.mark_layout_dirty();
    root.mark_layout_dirty().mark_layout_dirty();
}

// ── Phase 2-3 stubs compile ───────────────────────────────────────────────────

TEST(ObjectStubs, AddStyleStub) {
    Screen root;
    // No style infrastructure yet — just verify it compiles and is a no-op
    // (would crash with a real Style object in Phase 2)
    EXPECT_EQ(root.child_count(), 0u);  // side-effect: none
}

TEST(ObjectStubs, SendEventCallsOnEvent) {
    struct EventCapture : public Object {
        bool received = false;
        explicit EventCapture(Object* p) : Object(p) {}
        void on_event(Event& /*e*/) override { received = true; }
    };
    Screen root;
    auto& cap = root.create<EventCapture>();
    cap.send_event(EventCode::Clicked);
    EXPECT_TRUE(cap.received);
}

// ── screen() traversal ────────────────────────────────────────────────────────

TEST(ObjectScreen, ChildScreenIsSameAsRoot) {
    Screen root;
    auto& child = root.create<Widget>();
    EXPECT_EQ(child.screen(), &root);
}

TEST(ObjectScreen, GrandchildScreenIsSameAsRoot) {
    Screen root;
    auto& child = root.create<Widget>();
    auto& grand = child.create<Widget>();
    EXPECT_EQ(grand.screen(), &root);
}

TEST(ObjectScreen, OrphanWidgetHasNullScreen) {
    // A Widget with nullptr parent is its own root but not a Screen
    Widget w{nullptr};
    EXPECT_EQ(w.screen(), nullptr);
}
