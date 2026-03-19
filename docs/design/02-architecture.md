# 02 – Architecture Overview

## 1. Module Map

```
┌─────────────────────────────────────────────────────────────────────┐
│                          Application Layer                          │
│           (user widgets, screens, event handlers, themes)           │
└───────────────────────────┬─────────────────────────────────────────┘
                            │ import lvgl;
┌───────────────────────────▼─────────────────────────────────────────┐
│                       lvgl (module facade)                          │
│  re-exports: lvgl.core  lvgl.widgets  lvgl.misc  lvgl.drivers       │
└──┬───────────┬──────────┬──────────────┬──────────────┬─────────────┘
   │           │          │              │              │
   ▼           ▼          ▼              ▼              ▼
lvgl.core  lvgl.style  lvgl.anim  lvgl.widgets    lvgl.drivers
   │           │          │         (per widget)       │
   │     ┌─────┘   ┌──────┘              │        ┌────┴────────────┐
   │     │         │                     │        │                 │
   ▼     ▼         ▼                     ▼        ▼                 ▼
lvgl.misc  ←──────────────────── lvgl.display  lvgl.indev
(color, area, font, math, timer, log, alloc)
```

### Sub-module responsibilities

| Module | Exported symbols |
|---|---|
| `lvgl.misc` | `Color`, `Area`, `Point`, `Font`, `Timer`, `Tick`, `Log`, allocators |
| `lvgl.core` | `Object`, `Group`, `Screen`, `Observer`, `Subject` |
| `lvgl.style` | `Style`, `StyleSheet`, `Theme`, `Transition` |
| `lvgl.anim` | `Animation`, `AnimationTimeline`, easing functions |
| `lvgl.display` | `Display`, `DrawBuffer`, `Layer` |
| `lvgl.indev` | `InputDevice`, `Pointer`, `Encoder`, `Keypad` |
| `lvgl.widgets` | All concrete widget classes |
| `lvgl.drivers` | Platform driver traits / concept definitions |

## 2. Layer Dependency Rules

Allowed dependency directions (→ means "may depend on"):

```
drivers   → display, indev
widgets   → core, style, anim, misc
core      → style, anim, misc
style     → misc
anim      → misc
display   → misc
indev     → misc
misc      → (nothing inside lvgl)
```

**Circular dependencies are forbidden.**  The module system enforces this
statically.

## 3. Object Ownership Tree

```
Display (owns 1..N screens)
└── Screen : Object  (root of the widget tree)
    └── Container : Object
        ├── Label : Object
        ├── Button : Object
        │   └── Label : Object
        └── Slider : Object
```

Ownership is expressed via `std::vector<std::unique_ptr<Object>>` in each
parent.  Children are destroyed when the parent is destroyed (RAII).

External handles (`ObjectRef<T>`) are non-owning references that automatically
become null when the referent is deleted (via an intrusive validity bit).

## 4. Rendering Pipeline

```
lv::tick()  ──►  Invalidation marks on dirty Objects
                        │
                        ▼
            Display::refresh()  (called by tick or manually)
                        │
              ┌─────────┴──────────┐
              ▼                    ▼
       collect draw tasks    calculate clip areas
              │
              ▼
       DrawBuffer (std::mdspan<Pixel,2>)
              │
              ▼
       Renderer concept  ──►  SoftwareRenderer  (built-in)
                         └──►  CustomRenderer   (user-supplied)
              │
              ▼
       Display::flush_cb  (user driver callback)
```

## 5. Event Flow

```
InputDevice::poll()
      │  produces lv::InputState
      ▼
Indev resolver  (hit-test / focus resolution)
      │  creates lv::Event
      ▼
Object::dispatch_event()
      │
      ├─► event handlers (std::move_only_function)  [registered order]
      │
      └─► bubble / trickle (if flag set)
```

## 6. Configuration Architecture

```cpp
// User defines a Policy struct:
struct MyConfig {
    using ColorFormat  = lv::color::ARGB8888;
    using Allocator    = lv::ArenaAllocator<65536>;
    static constexpr bool UseFlexLayout    = true;
    static constexpr bool UseGridLayout    = true;
    static constexpr bool UseAnimations    = true;
    static constexpr int  MaxDisplays      = 1;
};

// Instantiate the library with that policy:
using LV = lv::Library<MyConfig>;
using Display = LV::Display;
```

## 7. Thread Safety Model

lvgl-cxx follows the same cooperative, single-threaded-per-display model
as upstream LVGL, with explicit concurrency primitives for multi-display
setups:

- Each `Display` owns a `std::jthread` running the tick/refresh loop.
- Cross-display communication uses `lv::async_call()` (an MPSC queue).
- All widget mutations from other threads must go through `lv::async_call()`.
- A `std::mutex`-based lock is available for platforms that require it
  (opt-in via policy).

## 8. Error Handling Strategy

| Severity | Mechanism |
|---|---|
| Programming errors (precondition violation) | `lv::assert()` → `std::terminate()` in debug; `[[assume]]` in release |
| Recoverable API errors | `std::expected<T, lv::Error>` return values |
| Resource exhaustion | `std::expected` with `lv::Error::OutOfMemory` |
| Driver / platform failures | `std::expected` propagated to caller |

Exceptions are **not used** (compatible with `-fno-exceptions` builds).
