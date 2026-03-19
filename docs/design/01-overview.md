# 01 – Overview & Design Goals

## 1. Purpose

`lvgl-cxx` is a **native C++23 redesign** of the LVGL embedded GUI library.
It is *not* a thin C++ wrapper; it is a **breaking-change rewrite** that
discards all C-language compatibility guarantees and leverages every relevant
C++23 feature to produce a safer, more expressive, and more maintainable API.

## 2. Relationship to Upstream LVGL

| Property | LVGL (C) | lvgl-cxx (C++23) |
|---|---|---|
| Language standard | C99 (C++ compatible via `extern "C"`) | C++23 |
| ABI compatibility | Stable C ABI | C++ ABI (no C ABI requirement) |
| Configuration | `lv_conf.h` macros | Compile-time policy types + concepts |
| Error handling | Return code `lv_result_t` | `std::expected<T, lv::Error>` |
| Callbacks | Raw function pointers + `void*` | `std::move_only_function<void(Event&)>` |
| Memory | `lv_malloc` / `lv_free` custom allocator | C++ Allocator concept + arena |
| Thread safety | Manual mutex (`lv_os`) | `std::mutex` / `std::jthread` RAII |
| Logging | `LV_LOG_*` macros | `std::print` / custom sink |
| Modules | `#include` headers | C++23 named modules (`import lvgl;`) |

lvgl-cxx tracks the **LVGL v9.5.0** feature set and widget catalog as its
functional baseline (latest release: [v9.5.0](https://github.com/lvgl/lvgl/releases/tag/v9.5.0),
published 2026-02-18).

## 3. Design Goals

### G1 – Zero overhead abstractions
Every abstraction must compile to code indistinguishable from hand-written C
at `-O2` or higher on ARM Cortex-M and similar embedded targets.  Templates
are preferred over virtual dispatch in the hot rendering path.

### G2 – Type-safe API throughout
No `void*` user-data slots, no untyped property bags, no raw function pointers
in the public API.  All type erasure is internal.

### G3 – Ownership clarity
Every widget has a single owner (its parent container or an RAII handle).
`std::unique_ptr`-style semantics ensure no accidental double-delete.

### G4 – Composable, fluent builder pattern
Widget construction uses method chaining:
```cpp
auto& lbl = parent.create<lv::Label>()
    .set_text("hello")
    .align(lv::Align::Center)
    .set_style(lv::Style{}.text_color(lv::Color::Red));
```

### G5 – Compile-time configurability
Display depth, color format, feature set, and allocator are **policy
parameters** resolved at compile time – no runtime branches for disabled
features.

### G6 – Embedded-first resource budget
The default configuration must fit in ≤ 128 kB Flash / 32 kB SRAM on
Cortex-M4.  All heap allocation goes through an injected allocator; a
fully static arena allocator is provided for bare-metal targets.

### G7 – Testability
Every subsystem is independently unit-testable.  The rendering backend is
injected via concept-constrained templates, enabling software-renderer stubs
in unit tests.

### G8 – Modules-first, headers as compatibility shim
The primary public API is exposed as a named C++23 module (`lvgl`).
Traditional `#include` headers remain available for toolchains that do not yet
support modules.

## 4. Non-Goals

- **C ABI compatibility** – lvgl-cxx does not expose a C-callable API.
- **Support for C++17 or earlier** – C++23 is the minimum.
- **Drop-in replacement** – Existing LVGL C applications require a migration
  step (see [10-porting-guide.md](10-porting-guide.md)).
- **Language bindings** – Python/Lua/MicroPython bindings are out of scope for
  v1.0.

## 5. Guiding C++23 Features

| Feature | Purpose in lvgl-cxx |
|---|---|
| **Modules** | Primary distribution unit, eliminates macro leakage |
| **`std::expected<T,E>`** | Monadic error propagation without exceptions |
| **`std::move_only_function`** | Non-copyable lambdas for event handlers |
| **Concepts & constraints** | Verify driver / allocator / renderer types |
| **Deducing `this`** | Fluent builder CRTP without template boilerplate |
| **`std::mdspan`** | 2-D pixel buffer views without ownership |
| **`std::flat_map` / `std::flat_set`** | Cache-friendly style property storage |
| **`std::generator`** | Lazy widget tree iterators |
| **`std::print` / `std::println`** | Structured logging |
| **`std::stacktrace`** | Debug assertion diagnostics |
| **`std::jthread`** | Tick thread, render thread RAII |
| **`if consteval`** | Compile-time vs runtime path selection |
| **Multidimensional `operator[]`** | Draw buffer pixel access |
| **`auto(x)` decay-copy** | Safe capture in animation closures |
| **`[[assume(expr)]]`** | Optimizer hints in inner loops |

## 6. Versioning Strategy

lvgl-cxx uses **semantic versioning**.  Major version 1 tracks LVGL v9.5.0
feature parity.  Major version 2 will introduce modules exclusively (header
shim dropped).

| lvgl-cxx | LVGL baseline | C++ standard |
|---|---|---|
| 1.x | [v9.5.0](https://github.com/lvgl/lvgl/releases/tag/v9.5.0) | C++23 |
| 2.x | v10.x (planned) | C++26 (planned) |
