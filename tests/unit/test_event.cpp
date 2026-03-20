// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// tests/unit/test_event.cpp
// Phase 3 unit tests covering:
//   • EventHandle registration / removal / RAII
//   • send_event → handler dispatch
//   • e.stop() halts subsequent handlers
//   • EventBubble flag → bubble to parent
//   • EventTrickle flag → dispatch to children
//   • Subject<T> / ObserverHandle RAII
//   • Group focus traversal (focus_next / focus_prev / wrap / no-wrap)
//   • Group::send_key dispatches EventCode::Key with key value

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "lvgl/core/event.hpp"
#include "lvgl/core/group.hpp"
#include "lvgl/core/object.hpp"
#include "lvgl/core/observer.hpp"
#include "lvgl/core/screen.hpp"

using namespace lv;

// ── Minimal concrete Object for testing ──────────────────────────────────────

struct Widget : public Object {
  explicit Widget(Object* p) : Object(p) {}
  std::vector<EventCode> received;
  void on_event(Event& e) override { received.push_back(e.code()); }
};

// ── EventHandle registration
// ──────────────────────────────────────────────────

TEST(EventHandle, HandlerFiresOnSendEvent) {
  Screen s;
  auto& w = s.create<Widget>();

  int count = 0;
  auto h = w.on(EventCode::Clicked, [&](Event&) { ++count; });

  w.send_event(EventCode::Clicked);
  EXPECT_EQ(count, 1);
  w.send_event(EventCode::Clicked);
  EXPECT_EQ(count, 2);
}

TEST(EventHandle, WrongCodeDoesNotFire) {
  Screen s;
  auto& w = s.create<Widget>();

  int count = 0;
  auto h = w.on(EventCode::Clicked, [&](Event&) { ++count; });

  w.send_event(EventCode::Pressed);
  EXPECT_EQ(count, 0);
}

TEST(EventHandle, MultipleHandlersSameCode) {
  Screen s;
  auto& w = s.create<Widget>();

  int a = 0, b = 0;
  auto h1 = w.on(EventCode::Clicked, [&](Event&) { ++a; });
  auto h2 = w.on(EventCode::Clicked, [&](Event&) { ++b; });

  w.send_event(EventCode::Clicked);
  EXPECT_EQ(a, 1);
  EXPECT_EQ(b, 1);
}

TEST(EventHandle, RaiiRemovesHandlerOnDestruction) {
  Screen s;
  auto& w = s.create<Widget>();

  int count = 0;
  {
    auto h = w.on(EventCode::Clicked, [&](Event&) { ++count; });
    w.send_event(EventCode::Clicked);
    EXPECT_EQ(count, 1);
  }  // h destroyed — handler removed

  w.send_event(EventCode::Clicked);
  EXPECT_EQ(count, 1);  // no change
}

TEST(EventHandle, ReleaseKeepsHandlerAlive) {
  Screen s;
  auto& w = s.create<Widget>();

  int count = 0;
  {
    auto h = w.on(EventCode::Clicked, [&](Event&) { ++count; });
    h.release();  // detach from RAII
  }  // h destroyed but handler should persist

  w.send_event(EventCode::Clicked);
  EXPECT_EQ(count, 1);  // handler still fires
}

TEST(EventHandle, ManualRemove) {
  Screen s;
  auto& w = s.create<Widget>();

  int count = 0;
  auto h = w.on(EventCode::Clicked, [&](Event&) { ++count; });

  w.send_event(EventCode::Clicked);
  EXPECT_EQ(count, 1);

  h.remove();
  EXPECT_FALSE(h.valid());

  w.send_event(EventCode::Clicked);
  EXPECT_EQ(count, 1);  // no change
}

TEST(EventHandle, ValidAfterRegistration) {
  Screen s;
  auto& w = s.create<Widget>();
  auto h = w.on(EventCode::Clicked, [](Event&) {});
  EXPECT_TRUE(h.valid());
}

TEST(EventHandle, InvalidAfterOwnerDestroyed) {
  Screen s;
  EventHandle h;
  {
    auto& w = s.create<Widget>();
    h = w.on(EventCode::Clicked, [](Event&) {});
    EXPECT_TRUE(h.valid());
    s.remove_child(w);
  }  // w destroyed
  EXPECT_FALSE(h.valid());
}

// ── e.stop() ─────────────────────────────────────────────────────────────────

TEST(EventStop, StopPreventsSubsequentHandlers) {
  Screen s;
  auto& w = s.create<Widget>();

  std::vector<int> called;
  auto h1 = w.on(EventCode::Clicked, [&](Event& e) {
    called.push_back(1);
    e.stop();
  });
  auto h2 = w.on(EventCode::Clicked, [&](Event&) { called.push_back(2); });

  w.send_event(EventCode::Clicked);

  ASSERT_EQ(called.size(), 1u);
  EXPECT_EQ(called[0], 1);
}

// ── EventTarget accessors
// ─────────────────────────────────────────────────────

TEST(EventTarget, TargetAndCurrentTargetAreSameForDirectSend) {
  Screen s;
  auto& w = s.create<Widget>();

  Object* target_seen = nullptr;
  Object* current_seen = nullptr;
  auto h = w.on(EventCode::Clicked, [&](Event& e) {
    target_seen = &e.target();
    current_seen = &e.current_target();
  });

  w.send_event(EventCode::Clicked);
  EXPECT_EQ(target_seen, &w);
  EXPECT_EQ(current_seen, &w);
}

// ── Bubbling
// ──────────────────────────────────────────────────────────────────

TEST(EventBubble, PropagatesFromChildToParentWhenFlagSet) {
  Screen s;
  auto& parent = s.create<Widget>();
  auto& child = parent.create<Widget>();

  // Enable bubbling on child
  child.add_flag(ObjFlags::EventBubble);

  std::vector<Object*> fire_order;
  auto hc = child.on(EventCode::Clicked,
                     [&](Event&) { fire_order.push_back(&child); });
  auto hp = parent.on(EventCode::Clicked, [&](Event& e) {
    fire_order.push_back(&e.current_target());
  });

  child.send_event(EventCode::Clicked);

  ASSERT_EQ(fire_order.size(), 2u);
  EXPECT_EQ(fire_order[0], &child);
  EXPECT_EQ(fire_order[1], &parent);
}

TEST(EventBubble, StopBubblingPreventsPropagation) {
  Screen s;
  auto& parent = s.create<Widget>();
  auto& child = parent.create<Widget>();

  child.add_flag(ObjFlags::EventBubble);

  int parent_count = 0;
  auto hc = child.on(EventCode::Clicked, [](Event& e) { e.stop_bubbling(); });
  auto hp = parent.on(EventCode::Clicked, [&](Event&) { ++parent_count; });

  child.send_event(EventCode::Clicked);
  EXPECT_EQ(parent_count, 0);
}

TEST(EventBubble, NoFlagNoBubbling) {
  Screen s;
  auto& parent = s.create<Widget>();
  auto& child = parent.create<Widget>();
  // EventBubble NOT set on child

  int parent_count = 0;
  auto hp = parent.on(EventCode::Clicked, [&](Event&) { ++parent_count; });

  child.send_event(EventCode::Clicked);
  EXPECT_EQ(parent_count, 0);
}

// ── Trickle
// ───────────────────────────────────────────────────────────────────

TEST(EventTrickle, PropagatesFromParentToChildrenWhenFlagSet) {
  Screen s;
  auto& parent = s.create<Widget>();
  auto& c1 = parent.create<Widget>();
  auto& c2 = parent.create<Widget>();

  parent.add_flag(ObjFlags::EventTrickle);

  std::vector<Object*> fire_order;
  auto hp = parent.on(EventCode::ValueChanged,
                      [&](Event&) { fire_order.push_back(&parent); });
  auto hc1 = c1.on(EventCode::ValueChanged, [&](Event& e) {
    fire_order.push_back(&e.current_target());
  });
  auto hc2 = c2.on(EventCode::ValueChanged, [&](Event& e) {
    fire_order.push_back(&e.current_target());
  });

  parent.send_event(EventCode::ValueChanged);

  ASSERT_GE(fire_order.size(), 3u);
  EXPECT_EQ(fire_order[0], &parent);  // own handler fires first
  EXPECT_EQ(fire_order[1], &c1);
  EXPECT_EQ(fire_order[2], &c2);
}

TEST(EventTrickle, StopPreventsChildDispatch) {
  Screen s;
  auto& parent = s.create<Widget>();
  auto& child = parent.create<Widget>();

  parent.add_flag(ObjFlags::EventTrickle);

  int child_count = 0;
  auto hp = parent.on(EventCode::Clicked, [](Event& e) { e.stop(); });
  auto hc = child.on(EventCode::Clicked, [&](Event&) { ++child_count; });

  parent.send_event(EventCode::Clicked);
  EXPECT_EQ(child_count, 0);
}

// ── Event key accessor
// ────────────────────────────────────────────────────────

TEST(EventKey, KeyExtractedFromParam) {
  Screen s;
  auto& w = s.create<Widget>();

  uint32_t received_key = 0;
  auto h = w.on(EventCode::Key, [&](Event& e) { received_key = e.key(); });

  w.send_event(EventCode::Key,
               reinterpret_cast<void*>(static_cast<uintptr_t>(42u)));
  EXPECT_EQ(received_key, 42u);
}

TEST(EventKey, KeyZeroForNonKeyEvent) {
  Screen s;
  auto& w = s.create<Widget>();

  uint32_t received_key = 99;
  auto h = w.on(EventCode::Clicked, [&](Event& e) { received_key = e.key(); });

  w.send_event(EventCode::Clicked);
  EXPECT_EQ(received_key, 0u);
}

// ── Subject<T> / ObserverHandle
// ───────────────────────────────────────────────

TEST(Subject, ImmediateCallOnSubscribe) {
  IntSubject sub{7};
  int last = 0;
  auto h = sub.subscribe([&](int v) { last = v; });
  EXPECT_EQ(last, 7);
}

TEST(Subject, NotifiesOnChange) {
  IntSubject sub{0};
  int last = 0;
  auto h = sub.subscribe([&](int v) { last = v; });
  EXPECT_EQ(last, 0);

  sub.set(42);
  EXPECT_EQ(last, 42);

  sub.set(100);
  EXPECT_EQ(last, 100);
}

TEST(Subject, NoNotifyWhenValueUnchanged) {
  IntSubject sub{5};
  int calls = 0;
  auto h = sub.subscribe([&](int) { ++calls; });
  EXPECT_EQ(calls, 1);  // immediate call

  sub.set(5);           // same value
  EXPECT_EQ(calls, 1);  // no extra notification
}

TEST(Subject, MultipleSubscribers) {
  IntSubject sub{0};
  int a = 0, b = 0;
  auto h1 = sub.subscribe([&](int v) { a = v; });
  auto h2 = sub.subscribe([&](int v) { b = v; });
  sub.set(99);
  EXPECT_EQ(a, 99);
  EXPECT_EQ(b, 99);
}

TEST(ObserverHandle, RaiiUnsubscribesOnDestruction) {
  IntSubject sub{0};
  int calls = 0;
  {
    auto h = sub.subscribe([&](int) { ++calls; });
    EXPECT_EQ(calls, 1);  // immediate
    sub.set(1);
    EXPECT_EQ(calls, 2);
  }  // h destroyed → unsubscribe

  sub.set(2);
  EXPECT_EQ(calls, 2);  // no further notification
}

TEST(ObserverHandle, ReleaseKeepsSubscriptionAlive) {
  IntSubject sub{0};
  int calls = 0;
  {
    auto h = sub.subscribe([&](int) { ++calls; });
    h.release();
  }  // h destroyed but subscription persists
  sub.set(1);
  EXPECT_EQ(calls, 2);  // initial + one change
}

TEST(ObserverHandle, ExplicitUnsubscribe) {
  IntSubject sub{0};
  int calls = 0;
  auto h = sub.subscribe([&](int) { ++calls; });
  EXPECT_EQ(calls, 1);

  h.unsubscribe();
  EXPECT_FALSE(h.valid());

  sub.set(1);
  EXPECT_EQ(calls, 1);  // no change
}

TEST(Subject, StringSubject) {
  StringSubject sub{"hello"};
  std::string last;
  auto h = sub.subscribe([&](const std::string& s) { last = s; });
  EXPECT_EQ(last, "hello");

  sub.set("world");
  EXPECT_EQ(last, "world");
}

TEST(Subject, ModifyNotifies) {
  IntSubject sub{5};
  int last = 0;
  auto h = sub.subscribe([&](int v) { last = v; });
  EXPECT_EQ(last, 5);

  sub.modify([](int& v) { v += 10; });
  EXPECT_EQ(last, 15);
}

// ── Group focus traversal
// ─────────────────────────────────────────────────────

TEST(Group, AddAndCount) {
  Screen s;
  auto& w1 = s.create<Widget>();
  auto& w2 = s.create<Widget>();

  Group g;
  g.add(w1);
  g.add(w2);
  EXPECT_EQ(g.obj_count(), 2u);
}

TEST(Group, FocusNextCyclesThroughMembers) {
  Screen s;
  auto& w1 = s.create<Widget>();
  auto& w2 = s.create<Widget>();
  auto& w3 = s.create<Widget>();

  Group g;
  g.add(w1);
  g.add(w2);
  g.add(w3);

  EXPECT_EQ(g.focused(), nullptr);

  g.focus_next();
  EXPECT_EQ(g.focused(), &w1);

  g.focus_next();
  EXPECT_EQ(g.focused(), &w2);

  g.focus_next();
  EXPECT_EQ(g.focused(), &w3);

  // wrap around
  g.focus_next();
  EXPECT_EQ(g.focused(), &w1);
}

TEST(Group, FocusPrevCyclesBackward) {
  Screen s;
  auto& w1 = s.create<Widget>();
  auto& w2 = s.create<Widget>();
  auto& w3 = s.create<Widget>();

  Group g;
  g.add(w1);
  g.add(w2);
  g.add(w3);

  g.focus_prev();
  EXPECT_EQ(g.focused(), &w3);

  g.focus_prev();
  EXPECT_EQ(g.focused(), &w2);

  g.focus_prev();
  EXPECT_EQ(g.focused(), &w1);

  // wrap around
  g.focus_prev();
  EXPECT_EQ(g.focused(), &w3);
}

TEST(Group, FocusedObjectGetsFocusedState) {
  Screen s;
  auto& w = s.create<Widget>();

  Group g;
  g.add(w);

  EXPECT_FALSE(w.has_state(ObjState::Focused));
  g.focus_next();
  EXPECT_TRUE(w.has_state(ObjState::Focused));
}

TEST(Group, PreviousFocusLosesFocusedState) {
  Screen s;
  auto& w1 = s.create<Widget>();
  auto& w2 = s.create<Widget>();

  Group g;
  g.add(w1);
  g.add(w2);

  g.focus_next();  // w1 focused
  EXPECT_TRUE(w1.has_state(ObjState::Focused));

  g.focus_next();  // w2 focused, w1 not
  EXPECT_FALSE(w1.has_state(ObjState::Focused));
  EXPECT_TRUE(w2.has_state(ObjState::Focused));
}

TEST(Group, NoWrapStopsAtEnd) {
  Screen s;
  auto& w1 = s.create<Widget>();
  auto& w2 = s.create<Widget>();

  Group g;
  g.add(w1);
  g.add(w2);
  g.set_wrap(false);

  g.focus_next();
  EXPECT_EQ(g.focused(), &w1);

  g.focus_next();
  EXPECT_EQ(g.focused(), &w2);

  g.focus_next();  // at end, no wrap
  EXPECT_EQ(g.focused(), &w2);
}

TEST(Group, FocusSpecificObject) {
  Screen s;
  auto& w1 = s.create<Widget>();
  auto& w2 = s.create<Widget>();
  auto& w3 = s.create<Widget>();

  Group g;
  g.add(w1);
  g.add(w2);
  g.add(w3);

  g.focus(w3);
  EXPECT_EQ(g.focused(), &w3);
  EXPECT_TRUE(w3.has_state(ObjState::Focused));
}

TEST(Group, RemoveAdjustsFocusedIndex) {
  Screen s;
  auto& w1 = s.create<Widget>();
  auto& w2 = s.create<Widget>();
  auto& w3 = s.create<Widget>();

  Group g;
  g.add(w1);
  g.add(w2);
  g.add(w3);

  g.focus(w2);
  EXPECT_EQ(g.focused(), &w2);

  g.remove(w2);
  EXPECT_EQ(g.obj_count(), 2u);
}

TEST(Group, RemoveAllClearsList) {
  Screen s;
  auto& w1 = s.create<Widget>();
  auto& w2 = s.create<Widget>();

  Group g;
  g.add(w1);
  g.add(w2);
  g.remove_all();
  EXPECT_EQ(g.obj_count(), 0u);
  EXPECT_EQ(g.focused(), nullptr);
}

TEST(Group, SendKeyDispatchesKeyEvent) {
  Screen s;
  auto& w = s.create<Widget>();

  Group g;
  g.add(w);
  g.focus_next();  // focus w

  uint32_t last_key = 0;
  auto h = w.on(EventCode::Key, [&](Event& e) { last_key = e.key(); });

  g.send_key(65u);  // 'A'
  EXPECT_EQ(last_key, 65u);
}

TEST(Group, FocusFreezePreventsFocusChange) {
  Screen s;
  auto& w1 = s.create<Widget>();
  auto& w2 = s.create<Widget>();

  Group g;
  g.add(w1);
  g.add(w2);
  g.focus_next();  // w1 focused
  g.focus_freeze(true);

  g.focus_next();  // should do nothing
  EXPECT_EQ(g.focused(), &w1);

  g.focus_freeze(false);
  g.focus_next();
  EXPECT_EQ(g.focused(), &w2);
}

TEST(Group, DefaultGroup) {
  EXPECT_EQ(Group::default_group(), nullptr);

  Group g;
  g.set_as_default();
  EXPECT_EQ(Group::default_group(), &g);

  // Destructor clears default group
  {
    Group g2;
    g2.set_as_default();
    EXPECT_EQ(Group::default_group(), &g2);
  }
  EXPECT_EQ(Group::default_group(), nullptr);
}

TEST(Group, OnFocusChangeCallback) {
  Screen s;
  auto& w1 = s.create<Widget>();
  auto& w2 = s.create<Widget>();

  Group g;
  g.add(w1);
  g.add(w2);

  int callback_count = 0;
  g.set_on_focus_change([&](Group&) { ++callback_count; });

  g.focus_next();  // fires callback
  EXPECT_EQ(callback_count, 1);

  g.focus_next();  // fires callback
  EXPECT_EQ(callback_count, 2);
}

TEST(Group, SwapMembers) {
  Screen s;
  auto& w1 = s.create<Widget>();
  auto& w2 = s.create<Widget>();

  Group g;
  g.add(w1);
  g.add(w2);

  EXPECT_EQ(g.obj_at(0), &w1);
  EXPECT_EQ(g.obj_at(1), &w2);

  g.swap(w1, w2);

  EXPECT_EQ(g.obj_at(0), &w2);
  EXPECT_EQ(g.obj_at(1), &w1);
}
