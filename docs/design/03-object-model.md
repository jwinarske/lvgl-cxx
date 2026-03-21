# 03 – Object / Widget Model

## 1. C → C++23 Mapping

| C (LVGL v9.5.0)                            | C++23 (lvgl-cxx)                                                         |
|--------------------------------------------|--------------------------------------------------------------------------|
| `lv_obj_t` opaque struct                   | `lv::Object` base class                                                  |
| `lv_obj_class_t` vtable struct             | C++ `virtual` dispatch + CRTP mixin                                      |
| `lv_obj_create(parent)`                    | `parent.create<WidgetType>(args...)`                                     |
| `lv_obj_delete(obj)`                       | RAII – destroy child via `parent.remove_child(ref)` or parent destructor |
| `lv_obj_add_flag` / `lv_obj_remove_flag`   | `obj.add_flag(Flag::Hidden)`                                             |
| `lv_obj_add_state` / `lv_obj_remove_state` | `obj.add_state(State::Pressed)`                                          |
| `lv_obj_set_pos`, `lv_obj_set_size`        | `obj.set_pos(x,y)`, `obj.set_size(w,h)`                                  |
| `lv_obj_align`                             | `obj.align(Align::Center, 0, 0)`                                         |
| `lv_obj_add_style`                         | `obj.add_style(style, selector)`                                         |
| `lv_obj_set_user_data` (void*)             | not present; use subclassing or lambda capture                           |
| `lv_obj_send_event`                        | `obj.send_event(EventCode::Clicked)`                                     |
| `lv_obj_add_event_cb`                      | `obj.on(EventCode::Clicked, handler)`                                    |
| `lv_obj_get_child_count`                   | `obj.child_count()`                                                      |
| `lv_obj_get_child`                         | `obj.child_at(index)` or range-for                                       |

## 2. Class Hierarchy

```
lv::Object                        (base – non-copyable, non-movable once parented)
├── lv::Container                 (scrollable box, layout root)
│   ├── lv::Screen                (root of a display)
│   ├── lv::Window
│   ├── lv::TileView
│   ├── lv::TabView
│   └── lv::Menu
├── lv::Label
├── lv::Button
│   └── lv::ImageButton
├── lv::Arc
├── lv::Bar
├── lv::Slider
│   └── lv::SpinBox
├── lv::Switch
├── lv::Checkbox
├── lv::Dropdown
├── lv::Roller
├── lv::Keyboard
├── lv::TextArea
├── lv::Image
│   └── lv::AnimImage
├── lv::Line
├── lv::Canvas
├── lv::Chart
├── lv::Table
├── lv::ButtonMatrix
├── lv::Scale
├── lv::Led
├── lv::List
├── lv::MsgBox
├── lv::Calendar
├── lv::Span
├── lv::Spinner
└── lv::ArcLabel
```

## 3. `Object` Public Interface Sketch

```cpp
namespace lv {

class Object {
public:
    // Non-copyable, non-movable (stable address in parent's child list)
    Object(const Object&)            = delete;
    Object& operator=(const Object&) = delete;
    Object(Object&&)                 = delete;
    Object& operator=(Object&&)      = delete;

    virtual ~Object();

    // ── Tree ────────────────────────────────────────────────────────
    [[nodiscard]] Object*        parent()       noexcept;
    [[nodiscard]] const Object*  parent() const noexcept;
    [[nodiscard]] std::size_t    child_count()  const noexcept;
    [[nodiscard]] Object&        child_at(std::size_t index);
    [[nodiscard]] const Object&  child_at(std::size_t index) const;

    // Range-based iteration over children
    [[nodiscard]] auto children()       noexcept; // returns a range<Object&>
    [[nodiscard]] auto children() const noexcept;

    // Create a child widget of type T, forwarding args to T's constructor
    template<std::derived_from<Object> T, typename... Args>
    T& create(Args&&... args);

    // Remove and destroy a direct child (by reference or index)
    void remove_child(Object& child);
    void remove_child(std::size_t index);
    void remove_all_children();

    // ── Flags ───────────────────────────────────────────────────────
    using Flags = ObjFlags;              // enum class ObjFlags : uint32_t

    Object& add_flag(Flags f)    noexcept;
    Object& remove_flag(Flags f) noexcept;
    Object& set_flag(Flags f, bool v) noexcept;
    [[nodiscard]] bool has_flag(Flags f)     const noexcept;
    [[nodiscard]] bool has_flag_any(Flags f) const noexcept;

    // ── States ──────────────────────────────────────────────────────
    using States = ObjState;             // enum class ObjState : uint16_t

    Object& add_state(States s)    noexcept;
    Object& remove_state(States s) noexcept;
    Object& set_state(States s, bool v) noexcept;
    [[nodiscard]] States state()                 const noexcept;
    [[nodiscard]] bool   has_state(States s)     const noexcept;

    // ── Geometry ────────────────────────────────────────────────────
    Object& set_pos(int32_t x, int32_t y) noexcept;
    Object& set_x(int32_t x)              noexcept;
    Object& set_y(int32_t y)              noexcept;
    Object& set_size(int32_t w, int32_t h) noexcept;
    Object& set_width(int32_t w)           noexcept;
    Object& set_height(int32_t h)          noexcept;
    Object& align(Align a,
                  int32_t x_ofs = 0,
                  int32_t y_ofs = 0) noexcept;
    Object& align_to(const Object& base, Align a,
                     int32_t x_ofs = 0, int32_t y_ofs = 0) noexcept;

    [[nodiscard]] int32_t x()      const noexcept;
    [[nodiscard]] int32_t y()      const noexcept;
    [[nodiscard]] int32_t width()  const noexcept;
    [[nodiscard]] int32_t height() const noexcept;
    [[nodiscard]] Area    bounds() const noexcept;

    // ── Scroll ──────────────────────────────────────────────────────
    Object& set_scrollbar_mode(ScrollbarMode m) noexcept;
    Object& set_scroll_dir(Dir d)               noexcept;
    void    scroll_to(int32_t x, int32_t y, AnimEnable anim) noexcept;
    void    scroll_by(int32_t dx, int32_t dy, AnimEnable anim) noexcept;

    // ── Styles ──────────────────────────────────────────────────────
    Object& add_style(const Style& s,
                      StyleSelector sel = StyleSelector::Default);
    Object& remove_style(const Style& s,
                         StyleSelector sel = StyleSelector::Default);
    Object& remove_all_styles();
    Object& set_style(StylePropId prop, StyleValue val,
                      StyleSelector sel = StyleSelector::Default);

    // Convenience typed style setters (generated from style property table)
    Object& set_style_bg_color(Color c,
                               StyleSelector sel = StyleSelector::Default);
    Object& set_style_text_color(Color c,
                                 StyleSelector sel = StyleSelector::Default);
    // ... (all ~100 style props follow the same pattern)

    // ── Events ──────────────────────────────────────────────────────
    using Handler = std::move_only_function<void(Event&)>;

    EventHandle on(EventCode code, Handler handler);
    void        remove_event(EventHandle h);
    void        send_event(EventCode code, void* param = nullptr);

    // ── Layout ──────────────────────────────────────────────────────
    Object& set_layout(LayoutId id) noexcept;
    Object& mark_layout_dirty()     noexcept;

    // ── Misc ────────────────────────────────────────────────────────
    [[nodiscard]] Display*       display()       noexcept;
    [[nodiscard]] const Display* display() const noexcept;
    [[nodiscard]] Screen*        screen()        noexcept;

    void invalidate()               noexcept;
    void invalidate_area(Area a)    noexcept;

protected:
    explicit Object(Object* parent);

    // Virtual extension points for subclasses
    virtual void on_create()                    {}
    virtual void on_delete()                    {}
    virtual void on_event(Event& e)             {}
    virtual void on_draw(DrawContext& ctx)       {}
    virtual void on_size_changed()              {}
    virtual void on_style_changed()             {}
    virtual void on_child_changed(Object& child){}

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace lv
```

## 4. Widget Creation Pattern

### Factory method on parent

```cpp
template<std::derived_from<Object> T, typename... Args>
T& Object::create(Args&&... args) {
    auto child = std::make_unique<T>(this, std::forward<Args>(args)...);
    T& ref = *child;
    children_.push_back(std::move(child));
    ref.on_create();
    return ref;
}
```

### Screen as root

```cpp
lv::Display display{480, 320};
lv::Screen& screen = display.active_screen();

auto& btn = screen.create<lv::Button>();
auto& lbl = btn.create<lv::Label>();
lbl.set_text("OK");
```

## 5. Flags and States as Bitmask Enum Classes

```cpp
namespace lv {

enum class ObjFlags : uint32_t {
    None             = 0,
    Hidden           = 1u << 0,
    Clickable        = 1u << 1,
    ClickFocusable   = 1u << 2,
    Checkable        = 1u << 3,
    Scrollable       = 1u << 4,
    ScrollElastic    = 1u << 5,
    ScrollMomentum   = 1u << 6,
    ScrollOne        = 1u << 7,
    ScrollChainH     = 1u << 8,
    ScrollChainV     = 1u << 9,
    ScrollOnFocus    = 1u << 10,
    SnappableX       = 1u << 11,
    PressLock        = 1u << 12,
    EventBubble      = 1u << 13,
    GestureBubble    = 1u << 14,
    AdvHittest       = 1u << 15,
    IgnoreLayout     = 1u << 16,
    Floating         = 1u << 17,
    OverflowVisible  = 1u << 18,
    EventTrickle     = 1u << 19,
    StateTrickle     = 1u << 20,
    User1            = 1u << 24,
    User2            = 1u << 25,
    User3            = 1u << 26,
    User4            = 1u << 27,
};
// bitwise operators auto-generated via CRTP helper
LVGL_DEFINE_ENUM_BITMASK(ObjFlags)

enum class ObjState : uint16_t {
    Default    = 0x0000,
    Checked    = 0x0001,
    Focused    = 0x0002,
    FocusKey   = 0x0004,
    Edited     = 0x0008,
    Hovered    = 0x0010,
    Pressed    = 0x0020,
    Scrolled   = 0x0040,
    Disabled   = 0x0080,
    User1      = 0x1000,
    User2      = 0x2000,
    User3      = 0x4000,
    User4      = 0x8000,
    Any        = 0xFFFF,
};
LVGL_DEFINE_ENUM_BITMASK(ObjState)

} // namespace lv
```

## 6. Non-owning Reference Handle

```cpp
namespace lv {

// Lightweight, nullable handle.  Becomes null automatically when the
// referenced object is destroyed (uses intrusive ref-counting on Object).
template<std::derived_from<Object> T = Object>
class ObjectRef {
public:
    ObjectRef() noexcept = default;
    explicit ObjectRef(T& obj) noexcept;

    [[nodiscard]] bool    valid()    const noexcept;
    [[nodiscard]] T*      get()            noexcept;
    [[nodiscard]] const T* get()     const noexcept;
    T&                    operator*()      ;
    T*                    operator->()    noexcept;

    explicit operator bool() const noexcept { return valid(); }
};

} // namespace lv
```

## 7. Custom Widget Example

```cpp
namespace app {

class TemperatureWidget : public lv::Object {
public:
    explicit TemperatureWidget(lv::Object* parent)
        : lv::Object(parent)
    {}

    void set_temperature(float celsius) {
        celsius_ = celsius;
        label_.set_text(std::format("{:.1f} °C", celsius_));
        invalidate();
    }

protected:
    void on_create() override {
        set_size(lv::SizeContent, lv::SizeContent);
        label_ = create<lv::Label>().align(lv::Align::Center);
    }

private:
    lv::ObjectRef<lv::Label> label_;
    float celsius_ = 0.f;
};

} // namespace app
```
