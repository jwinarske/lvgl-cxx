# 04 – Style System Redesign

## 1. C → C++23 Mapping

| C (LVGL v9.5.0) | C++23 (lvgl-cxx) |
|---|---|
| `lv_style_t` struct | `lv::Style` value type |
| `lv_style_prop_t` enum | `lv::StyleProp<Tag>` typed descriptors |
| `lv_style_value_t` union | `std::variant<int32_t, Color, Font*, …>` |
| `lv_style_init` / `lv_style_reset` | Constructor / destructor |
| `lv_style_set_*` free functions | `style.set(StyleProp::BgColor, …)` |
| `lv_obj_add_style` | `obj.add_style(style, selector)` |
| `lv_style_selector_t` | `lv::StyleSelector` bitmask (state + part) |
| `lv_theme_t` struct | `lv::Theme` abstract base class |
| `lv_style_transition_dsc_t` | `lv::Transition` value type |

## 2. Style Value Type

All ~100 style properties share a discriminated union:

```cpp
namespace lv {

using StyleValue = std::variant<
    std::monostate,      // "not set"
    int32_t,             // coords, radii, opacities, enums
    Color,               // background, border, text colours
    ColorFilter,         // gradient descriptor
    const Font*,         // font pointer (non-owning)
    const void*,         // image source (non-owning)
    Transition           // animation descriptor for state transitions
>;

} // namespace lv
```

## 3. Property Descriptors

Each property is a `constexpr` tag type, eliminating stringly-typed or
integer-indexed property lookup:

```cpp
namespace lv::prop {

// Each tag carries its expected value type as a nested alias.
struct BgColor        { using value_type = Color;   };
struct BgOpacity      { using value_type = int32_t; };
struct BgGrad         { using value_type = ColorFilter; };
struct BorderColor    { using value_type = Color;   };
struct BorderWidth    { using value_type = int32_t; };
struct BorderOpacity  { using value_type = int32_t; };
struct BorderSide     { using value_type = BorderSide_e; };
struct TextColor      { using value_type = Color;   };
struct TextFont       { using value_type = const Font*; };
struct TextLineSpacing{ using value_type = int32_t; };
struct TextLetterSpacing{ using value_type = int32_t; };
struct TextAlign      { using value_type = TextAlign_e; };
struct Radius         { using value_type = int32_t; };
struct PadLeft        { using value_type = int32_t; };
struct PadRight       { using value_type = int32_t; };
struct PadTop         { using value_type = int32_t; };
struct PadBottom      { using value_type = int32_t; };
struct PadRow         { using value_type = int32_t; };
struct PadColumn      { using value_type = int32_t; };
struct Width          { using value_type = int32_t; };
struct Height         { using value_type = int32_t; };
struct MinWidth       { using value_type = int32_t; };
struct MinHeight      { using value_type = int32_t; };
struct MaxWidth       { using value_type = int32_t; };
struct MaxHeight      { using value_type = int32_t; };
struct Opacity        { using value_type = int32_t; };
struct TransformWidth { using value_type = int32_t; };
struct TransformHeight{ using value_type = int32_t; };
struct TransformScaleX{ using value_type = int32_t; };
struct TransformScaleY{ using value_type = int32_t; };
struct TransformRotation{ using value_type = int32_t; };
struct Transition     { using value_type = lv::Transition; };
struct ImgSrc         { using value_type = const void*; };
struct ImgRecolor     { using value_type = Color; };
struct ImgRecolorOpa  { using value_type = int32_t; };
// … (all ~100 properties follow the same pattern)

} // namespace lv::prop
```

### Concept for property tags

```cpp
namespace lv {

template<typename P>
concept StyleProperty = requires {
    typename P::value_type;
};

} // namespace lv
```

## 4. `Style` Class

```cpp
namespace lv {

class Style {
public:
    Style() noexcept = default;
    Style(const Style&);
    Style(Style&&) noexcept;
    Style& operator=(const Style&);
    Style& operator=(Style&&) noexcept;
    ~Style();

    // Typed setter – returns *this for chaining
    template<StyleProperty P>
    Style& set(P, typename P::value_type value);

    // Typed getter – returns nullopt if property not set
    template<StyleProperty P>
    [[nodiscard]] std::optional<typename P::value_type> get(P) const noexcept;

    // Remove a single property
    template<StyleProperty P>
    Style& remove(P) noexcept;

    void reset() noexcept;

    [[nodiscard]] bool empty() const noexcept;

    // Transition for a list of properties
    template<StyleProperty... Ps>
    Style& set_transition(Ps..., Transition t);

private:
    // std::flat_map gives sorted, cache-friendly storage.
    std::flat_map<uint16_t, StyleValue> props_;
};

// Fluent usage example:
//   Style s;
//   s.set(prop::BgColor{}, Color::from_hex(0x2196F3))
//    .set(prop::Radius{}, 8)
//    .set(prop::PadLeft{}, 12)
//    .set(prop::TextColor{}, Color::White);

} // namespace lv
```

## 5. Style Selector

Encodes `(part, state)` as a bitmask – replaces `lv_style_selector_t`:

```cpp
namespace lv {

enum class Part : uint32_t {
    Main       = 0x000000,
    Scrollbar  = 0x010000,
    Indicator  = 0x020000,
    Knob       = 0x030000,
    Selected   = 0x040000,
    Items      = 0x050000,
    Cursor     = 0x060000,
    // widget-specific parts continue…
    Any        = 0x0F0000,
};

// StyleSelector = Part | ObjState bitmask
struct StyleSelector {
    Part     part  = Part::Main;
    ObjState state = ObjState::Default;

    static constexpr StyleSelector Default{};
    static constexpr StyleSelector Pressed{ Part::Main, ObjState::Pressed };
    static constexpr StyleSelector Focused{ Part::Main, ObjState::Focused };
    static constexpr StyleSelector Checked{ Part::Main, ObjState::Checked };
    static constexpr StyleSelector Disabled{ Part::Main, ObjState::Disabled };
};

} // namespace lv
```

## 6. Theme

```cpp
namespace lv {

class Theme {
public:
    virtual ~Theme() = default;

    // Called once for each newly created object.
    // Implementations call obj.add_style(…) to apply default styles.
    virtual void apply(Object& obj) = 0;

    // Optional: provide a colour palette for other subsystems
    virtual Color primary_color()   const noexcept { return Color::Black; }
    virtual Color secondary_color() const noexcept { return Color::Black; }
};

} // namespace lv
```

## 7. Transition

```cpp
namespace lv {

struct Transition {
    uint32_t duration_ms  = 300;
    uint32_t delay_ms     = 0;
    EasingFn easing       = Easing::Linear;

    // Properties affected by this transition (type-safe list)
    // Usage:  Transition{}.for_props(prop::BgColor{}, prop::Opacity{})
    template<StyleProperty... Ps>
    Transition& for_props(Ps...);
};

} // namespace lv
```

## 8. StyleSheet (per-object cascading list)

Internally each `Object` holds a `StyleSheet` – an ordered list of
`(Style*, StyleSelector)` pairs.  Resolution walks the list from most- to
least-recently added, stopping at the first match per property:

```cpp
namespace lv {

class StyleSheet {
public:
    void add(const Style& s, StyleSelector sel);
    void remove(const Style& s, StyleSelector sel);
    void remove_all();

    template<StyleProperty P>
    [[nodiscard]] typename P::value_type
    resolve(P prop, ObjState state, Part part,
            const StyleSheet* parent = nullptr) const noexcept;
};

} // namespace lv
```
