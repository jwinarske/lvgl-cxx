# 08 – Configuration System

## 1. Overview

LVGL v9.5.0 uses a single 2000-line `lv_conf.h` macro file.  lvgl-cxx replaces
this entirely with a **compile-time policy type** that satisfies a concept.
No macros leak into user code.

## 2. Policy Concept

```cpp
namespace lv {

template<typename C>
concept LibraryConfig = requires {
    // Color format tag type
    typename C::ColorFormat;
    requires ColorFormatTag<typename C::ColorFormat>;

    // Allocator for the library's own heap use
    typename C::Allocator;
    requires LvAllocator<typename C::Allocator>;

    // Renderer type
    typename C::Renderer;

    // Scalar feature flags (constexpr bool)
    { C::UseFlexLayout         } -> std::convertible_to<bool>;
    { C::UseGridLayout         } -> std::convertible_to<bool>;
    { C::UseAnimations         } -> std::convertible_to<bool>;
    { C::UseObserver           } -> std::convertible_to<bool>;
    { C::UseShadow             } -> std::convertible_to<bool>;
    { C::UseOutline            } -> std::convertible_to<bool>;
    { C::UseLayerTransform     } -> std::convertible_to<bool>;
    { C::UseScrollAnim         } -> std::convertible_to<bool>;

    // Integer limits
    { C::MaxDisplays           } -> std::convertible_to<int>;
    { C::MaxIndevDevices       } -> std::convertible_to<int>;
    { C::MaxAnimations         } -> std::convertible_to<int>;
    { C::MaxStyleTransitions   } -> std::convertible_to<int>;
    { C::DrawBufPercentage     } -> std::convertible_to<int>;

    // DPI hint
    { C::DefaultDpi            } -> std::convertible_to<int>;
};

} // namespace lv
```

## 3. Default Configuration

```cpp
namespace lv {

struct DefaultConfig {
    // Color
    using ColorFormat = color::ARGB8888;

    // Memory
    using Allocator   = SystemAllocator;

    // Rendering
    using Renderer    = SoftwareRenderer;

    // Layout engines
    static constexpr bool UseFlexLayout       = true;
    static constexpr bool UseGridLayout       = true;

    // Optional subsystems
    static constexpr bool UseAnimations       = true;
    static constexpr bool UseObserver         = true;
    static constexpr bool UseShadow           = true;
    static constexpr bool UseOutline          = true;
    static constexpr bool UseLayerTransform   = true;
    static constexpr bool UseScrollAnim       = true;

    // Resource limits
    static constexpr int MaxDisplays          = 4;
    static constexpr int MaxIndevDevices      = 4;
    static constexpr int MaxAnimations        = 64;
    static constexpr int MaxStyleTransitions  = 6;
    static constexpr int DrawBufPercentage    = 10;  // % of screen height

    // UI metrics
    static constexpr int DefaultDpi           = 130;
};

static_assert(LibraryConfig<DefaultConfig>);

} // namespace lv
```

## 4. Minimal Embedded Configuration Example

```cpp
struct EmbeddedConfig {
    using ColorFormat = lv::color::RGB565;
    using Allocator   = lv::ArenaAllocator<49152>;  // 48 kB arena
    using Renderer    = lv::SoftwareRenderer;

    static constexpr bool UseFlexLayout       = true;
    static constexpr bool UseGridLayout       = false;
    static constexpr bool UseAnimations       = false;
    static constexpr bool UseObserver         = false;
    static constexpr bool UseShadow           = false;
    static constexpr bool UseOutline          = false;
    static constexpr bool UseLayerTransform   = false;
    static constexpr bool UseScrollAnim       = false;

    static constexpr int MaxDisplays          = 1;
    static constexpr int MaxIndevDevices      = 2;
    static constexpr int MaxAnimations        = 8;
    static constexpr int MaxStyleTransitions  = 2;
    static constexpr int DrawBufPercentage    = 10;
    static constexpr int DefaultDpi           = 100;
};
static_assert(lv::LibraryConfig<EmbeddedConfig>);

using LV      = lv::Library<EmbeddedConfig>;
using Display = LV::Display;
using Label   = LV::Label;
```

## 5. Library Instantiation

```cpp
namespace lv {

// Primary template – brings all sub-types into scope under a policy-specific
// namespace alias.
template<LibraryConfig C = DefaultConfig>
struct Library {
    using Config         = C;
    using Display        = detail::Display<C>;
    using Screen         = detail::Screen<C>;
    using Object         = detail::Object<C>;
    using Style          = detail::Style<C>;
    // … one alias per exported type
    using Label          = detail::Label<C>;
    using Button         = detail::Button<C>;
    using Slider         = detail::Slider<C>;
    // …
};

// For the default config, unqualified names work directly:
// lv::Display, lv::Label, etc.
using DefaultLibrary = Library<DefaultConfig>;

} // namespace lv
```

## 6. Feature-Gated Code in Widgets

Conditional compilation uses `if constexpr` instead of `#ifdef`:

```cpp
// Inside Button::on_draw() (simplified):
template<LibraryConfig C>
void Button<C>::on_draw(Layer& layer) {
    if constexpr (C::UseShadow) {
        draw_shadow(layer);
    }
    draw_background(layer);
    draw_border(layer);
    if constexpr (C::UseOutline) {
        draw_outline(layer);
    }
}
```

## 7. Color Format Tags

```cpp
namespace lv::color {

struct RGB565 {
    struct pixel_type {
        uint16_t r : 5;
        uint16_t g : 6;
        uint16_t b : 5;
    };
    static constexpr int bits_per_pixel = 16;
    static constexpr bool has_alpha     = false;
};

struct ARGB8888 {
    struct pixel_type {
        uint8_t b, g, r, a;
    };
    static constexpr int bits_per_pixel = 32;
    static constexpr bool has_alpha     = true;
};

struct RGB888 {
    struct pixel_type {
        uint8_t b, g, r;
    };
    static constexpr int bits_per_pixel = 24;
    static constexpr bool has_alpha     = false;
};

struct L8 {
    using pixel_type = uint8_t;
    static constexpr int bits_per_pixel = 8;
    static constexpr bool has_alpha     = false;
};

template<typename T>
concept ColorFormatTag = requires {
    typename T::pixel_type;
    { T::bits_per_pixel } -> std::convertible_to<int>;
    { T::has_alpha      } -> std::convertible_to<bool>;
};

} // namespace lv::color
```

## 8. Meson Integration

Feature options map directly to config policy members (see
[`meson.options`](../../meson.options)):

```ini
option('color_format',   type: 'combo',
       choices: ['argb8888','rgb888','rgb565','l8'],
       value: 'argb8888')
option('use_flex',       type: 'boolean', value: true)
option('use_grid',       type: 'boolean', value: true)
option('use_animations', type: 'boolean', value: true)
option('use_shadow',     type: 'boolean', value: true)
option('use_observer',   type: 'boolean', value: true)
option('max_displays',   type: 'integer', value: 4)
```

The top-level `meson.build` generates a `lvgl_config.hpp` header from these
options, which is then `#include`d into the policy type definition.
