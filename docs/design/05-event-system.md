# 05 – Event System Redesign

## 1. C → C++23 Mapping

| C (LVGL v9.5.0)                                   | C++23 (lvgl-cxx)                             |
|---------------------------------------------------|----------------------------------------------|
| `lv_event_t` struct                               | `lv::Event` value type passed by reference   |
| `lv_event_code_t` enum                            | `lv::EventCode` enum class                   |
| `lv_event_cb_t` function pointer                  | `std::move_only_function<void(Event&)>`      |
| `lv_obj_add_event_cb(obj, cb, filter, user_data)` | `obj.on(EventCode, handler)` → `EventHandle` |
| `lv_obj_remove_event_dsc`                         | `obj.remove_event(handle)`                   |
| `lv_obj_send_event`                               | `obj.send_event(EventCode)`                  |
| `lv_event_get_target_obj`                         | `e.target<T>()`                              |
| `lv_event_get_indev`                              | `e.indev()`                                  |
| `lv_event_get_key`                                | `e.key()`                                    |
| `lv_event_get_param`                              | replaced by typed accessors per event code   |
| `lv_event_stop_processing`                        | `e.stop()`                                   |
| `lv_event_stop_bubbling`                          | `e.stop_bubbling()`                          |

## 2. `EventCode` Enum Class

```cpp
namespace lv {

enum class EventCode : uint32_t {
    // Input events
    Pressed,
    PressLost,
    ShortClicked,
    LongPressed,
    LongPressedRepeat,
    Clicked,
    Released,
    Scroll,
    ScrollBegin,
    ScrollEnd,
    ScrollThrowBegin,
    Gesture,
    Key,
    Rotary,
    Focused,
    Defocused,

    // Widget state events
    ValueChanged,
    Insert,
    Refresh,
    Ready,
    Cancel,
    CheckedChanged,

    // Layout / draw events
    Draw,
    DrawMain,
    DrawPost,
    DrawTaskAdded,
    RefreshExtDrawSize,
    GetSelfSize,
    HitTest,
    CoverCheck,

    // Lifecycle events
    Created,
    Deleted,
    ChildCreated,
    ChildDeleted,
    ScreenLoaded,
    ScreenUnloaded,
    ScreenLoadStart,
    ScreenUnloadStart,
    SizeChanged,
    StyleChanged,
    LayoutChanged,
    StateChanged,

    // User-defined range
    User1 = 0x100,
    User2,
    User3,
    User4,
    // extend as needed
};

} // namespace lv
```

## 3. `Event` Class

```cpp
namespace lv {

class Event {
public:
    // Who fired the event
    [[nodiscard]] Object&       target()         noexcept;
    [[nodiscard]] const Object& target()   const noexcept;

    // Current handler's perspective (may differ if bubbling)
    [[nodiscard]] Object&       current_target()       noexcept;
    [[nodiscard]] const Object& current_target() const noexcept;

    // Typed target cast – asserts in debug if wrong type
    template<std::derived_from<Object> T>
    [[nodiscard]] T& target();

    [[nodiscard]] EventCode code()  const noexcept;

    // Typed parameter accessors (only valid for relevant event codes)
    [[nodiscard]] uint32_t          key()             const;  // EventCode::Key
    [[nodiscard]] int32_t           rotary_diff()     const;  // EventCode::Rotary
    [[nodiscard]] const InputDevice* indev()          const;  // pointer/encoder events
    [[nodiscard]] DrawContext*       layer()          const;  // draw events
    [[nodiscard]] ObjState           prev_state()     const;  // StateChanged
    [[nodiscard]] const Area*        old_size()       const;  // SizeChanged
    [[nodiscard]] HitTestInfo*       hit_test_info()  const;  // HitTest
    [[nodiscard]] Animation*         scroll_anim()    const;  // ScrollBegin

    // Flow control
    void stop()          noexcept;  // stop further handlers on this object
    void stop_bubbling() noexcept;  // prevent bubble to parent
    [[nodiscard]] bool is_stopped()           const noexcept;
    [[nodiscard]] bool is_bubbling_stopped()  const noexcept;

    // Draw helpers (only valid in draw events)
    void set_ext_draw_size(int32_t size) noexcept;
    void set_cover_res(CoverResult res)  noexcept;

private:
    friend class Object;
    explicit Event(Object& target, EventCode code, void* param);

    Object*    target_;
    Object*    current_target_;
    EventCode  code_;
    void*      param_;
    bool       stopped_         = false;
    bool       bubble_stopped_  = false;
};

} // namespace lv
```

## 4. Handler Registration

### `EventHandle` – RAII token

```cpp
namespace lv {

// Returned by obj.on(…).  Destroying the handle removes the handler
// automatically (unless release() is called to detach from RAII).
class [[nodiscard]] EventHandle {
public:
    EventHandle() noexcept = default;
    ~EventHandle();                        // auto-removes handler

    EventHandle(EventHandle&&) noexcept;
    EventHandle& operator=(EventHandle&&) noexcept;

    void release() noexcept;              // detach: handler persists until obj deleted
    void remove()  noexcept;              // explicitly remove now

    [[nodiscard]] bool valid() const noexcept;
    explicit operator bool()   const noexcept { return valid(); }

private:
    friend class Object;
    // …
};

} // namespace lv
```

### Registration on `Object`

```cpp
// Single-event handler
EventHandle obj.on(EventCode code,
                   std::move_only_function<void(Event&)> handler);

// Multi-event (any of the listed codes)
EventHandle obj.on(std::initializer_list<EventCode> codes,
                   std::move_only_function<void(Event&)> handler);

// Catch-all handler
EventHandle obj.on_any(std::move_only_function<void(Event&)> handler);
```

### Usage examples

```cpp
// Lambda – handler removed when handle goes out of scope
{
    auto h = btn.on(lv::EventCode::Clicked, [](lv::Event& e) {
        lv::println("clicked!");
    });
} // h destroyed → handler auto-removed

// Persistent handler (release ownership)
btn.on(lv::EventCode::ValueChanged, [](lv::Event& e) {
    auto& slider = e.target<lv::Slider>();
    lv::println("value = {}", slider.value());
}).release();

// Member function via lambda capture
btn.on(lv::EventCode::Clicked, [this](lv::Event&) {
    this->on_button_clicked();
}).release();
```

## 5. Observer / Subject (reactive data binding)

Replaces `lv_observer.h` with a type-safe publish-subscribe mechanism.

```cpp
namespace lv {

// Subject<T> holds a value; observers are notified on change.
template<typename T>
class Subject {
public:
    explicit Subject(T initial = T{});

    void set(T value);
    [[nodiscard]] const T& get() const noexcept;

    // Bind: whenever value changes, run fn(new_value)
    // Returns RAII handle – destroy to unsubscribe.
    [[nodiscard]] ObserverHandle
    subscribe(std::move_only_function<void(const T&)> fn);

    // Convenience: bind subject value directly to a style property on obj
    template<StyleProperty P>
    [[nodiscard]] ObserverHandle
    bind_style(Object& obj, P prop,
               StyleSelector sel = StyleSelector::Default);

private:
    T value_;
    std::vector</* subscriber list */> subscribers_;
};

// Specialisations for common types
using IntSubject    = Subject<int32_t>;
using FloatSubject  = Subject<float>;
using StringSubject = Subject<std::string>;
using ColorSubject  = Subject<Color>;

} // namespace lv
```

### Binding example

```cpp
lv::IntSubject temperature{20};

auto& lbl = screen.create<lv::Label>();

// Auto-update label text whenever temperature changes
auto binding = temperature.subscribe([&lbl](int32_t t) {
    lbl.set_text(std::format("{} °C", t));
});

// Somewhere else:
temperature.set(25);   // lbl updates automatically
```

## 6. Custom Event Codes

User-defined event codes avoid numeric collisions:

```cpp
namespace app {

inline constexpr lv::EventCode EvtDataReady{
    static_cast<uint32_t>(lv::EventCode::User1) + 0
};
inline constexpr lv::EventCode EvtConnected{
    static_cast<uint32_t>(lv::EventCode::User1) + 1
};

} // namespace app

// Usage:
widget.send_event(app::EvtDataReady);
widget.on(app::EvtDataReady, [](lv::Event&) { /* … */ }).release();
```
