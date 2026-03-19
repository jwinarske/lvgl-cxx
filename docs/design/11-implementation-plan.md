# 11 – Phased Implementation Plan

## 1. Overview

The implementation is divided into six phases, each producing a testable
milestone.  All phases use **Meson** as the build system and target C++23.

| Phase | Milestone | Estimated effort |
|---|---|---|
| 0 | Scaffolding, build system, CI | 1 week |
| 1 | Core: Object tree, flags, states, geometry | 3 weeks |
| 2 | Style system, themes, transitions | 3 weeks |
| 3 | Event system, observer, groups | 2 weeks |
| 4 | Display, draw buffer, software renderer | 4 weeks |
| 5 | Widget library (all 30+ widgets) | 8 weeks |
| 6 | Drivers, platform abstractions, integration tests | 3 weeks |

---

## Phase 0 – Scaffolding (Week 1)

### Deliverables
- `meson.build` + `meson.options` with all feature options
- Cross-file templates: `cross/arm-none-eabi.ini`, `cross/riscv32.ini`
- `include/lvgl/` directory tree with empty stubs (`.hpp` files with just
  license header + `#pragma once`)
- `tests/` Meson test infrastructure with a "hello world" test
- CI pipeline (GitHub Actions) running on Linux, macOS, Windows with GCC-14
  and Clang-17
- `LICENCE.txt` and `README.md`

### Checklist
- [x] `meson.build` compiles an empty static library
- [x] `meson test` runs and passes the stub test
- [x] CI green on all three platforms

---

## Phase 1 – Core Object Tree (Weeks 2–4)

### Deliverables
- `include/lvgl/core/object.hpp` — `lv::Object` base class
- `include/lvgl/core/screen.hpp` — `lv::Screen`
- `include/lvgl/misc/area.hpp` — `lv::Area`, `lv::Point`
- `include/lvgl/misc/color.hpp` — `lv::Color`, color format tags
- `src/core/object.cpp` implementation
- Unit tests: tree construction, flag/state mutation, geometry, child iteration

### Key decisions settled in this phase
- Layout of `Object::Impl` (pimpl pattern)
- `ObjectRef<T>` validity tracking mechanism
- Allocator plumbing through the tree

### Checklist
- [x] `Object::create<T>()` constructs and parents children
- [x] Parent destructor recursively destroys children
- [x] `ObjectRef<T>` nullifies on child destroy
- [x] Flags and states are bitwise correct
- [x] `set_pos`, `set_size`, `align` write to Impl fields
- [x] `children()` returns a correct forward range
- [x] All Phase-1 unit tests pass

---

## Phase 2 – Style System (Weeks 5–7)

### Deliverables
- `include/lvgl/core/style.hpp` — `lv::Style`, `lv::StyleSelector`
- `include/lvgl/core/theme.hpp` — `lv::Theme` abstract base
- `include/lvgl/core/transition.hpp` — `lv::Transition`
- `src/core/style.cpp`, `src/core/style_sheet.cpp`
- Built-in themes: `DefaultTheme`, `MonoTheme`
- All ~100 `prop::*` tag structs generated from a Meson code-gen script
- Unit tests: property resolution, cascade order, selector priority, theme
  application

### Checklist
- [ ] `Style::set` / `Style::get` round-trip for all property types
- [ ] Cascade resolves state-specific style before default style
- [ ] Part selector restricts style to correct sub-part
- [ ] Transition descriptor stored and retrieved correctly
- [ ] `DefaultTheme::apply()` styles a freshly created `Button`
- [ ] All Phase-2 unit tests pass

---

## Phase 3 – Events, Observer, Groups (Weeks 8–9)

### Deliverables
- `include/lvgl/core/event.hpp` — `lv::Event`, `lv::EventCode`, `lv::EventHandle`
- `include/lvgl/core/observer.hpp` — `lv::Subject<T>`, `lv::ObserverHandle`
- `include/lvgl/core/group.hpp` — `lv::Group`
- `src/core/event.cpp`, `src/core/observer.cpp`, `src/core/group.cpp`
- Unit tests: handler registration/removal, bubbling, trickle, RAII handles,
  observer subscribe/unsubscribe, group focus traversal

### Checklist
- [ ] `obj.on(code, fn)` registers handler; fires on `send_event`
- [ ] Destroying `EventHandle` removes the handler
- [ ] `e.stop()` prevents subsequent handlers from running
- [ ] Bubbling propagates to parent when `EventBubble` flag set
- [ ] Trickle propagates to children when `EventTrickle` flag set
- [ ] `Subject<T>::set()` notifies all subscribers
- [ ] `ObserverHandle` RAII unsubscribes on destroy
- [ ] `Group::focus_next()` / `focus_prev()` cycle correctly
- [ ] All Phase-3 unit tests pass

---

## Phase 4 – Display & Rendering (Weeks 10–13)

### Deliverables
- `include/lvgl/core/display.hpp` — `lv::Display<Config>`
- `include/lvgl/core/draw_buffer.hpp` — `lv::DrawBuffer<Pixel>`
- `include/lvgl/core/layer.hpp` — `lv::Layer`
- `include/lvgl/draw/descriptors.hpp` — all draw descriptor structs
- `include/lvgl/draw/software_renderer.hpp` — `lv::SoftwareRenderer`
- `src/draw/sw/` software renderer implementation (ported from LVGL v9.5.0 C source,
  re-expressed in C++23)
- `include/lvgl/misc/anim.hpp` — `lv::Animation`, easing functions
- `include/lvgl/tick/tick.hpp` — `lv::tick_increment`, `lv::Ticker`
- Platform integration tests: render a `Label` + `Button` to an in-memory
  buffer and compare against a golden image

### Checklist
- [ ] `Display` constructor accepts `flush_cb` satisfying concept
- [ ] `DrawBuffer[row, col]` multidimensional subscript compiles and is
       bounds-checked in debug
- [ ] `SoftwareRenderer::execute` renders a `RectTask` to correct pixels
- [ ] Dirty-region invalidation marks only changed areas
- [ ] `Animation` runs exec callback over time with correct easing
- [ ] Flush callback is called after each refresh cycle
- [ ] Golden-image comparison tests pass (PNG diff tolerance < 1%)
- [ ] All Phase-4 unit tests pass

---

## Phase 5 – Widget Library (Weeks 14–21)

Widget implementation order (highest priority first):

| Week | Widgets |
|---|---|
| 14 | `Label`, `Button`, `Container` |
| 15 | `Slider`, `Bar`, `Arc` |
| 16 | `Switch`, `Checkbox`, `Led` |
| 17 | `Dropdown`, `Roller`, `TextArea` |
| 18 | `Image`, `AnimImage`, `ImageButton` |
| 19 | `Chart`, `Table`, `ButtonMatrix` |
| 20 | `Scale`, `SpinBox`, `Spinner`, `ArcLabel` |
| 21 | `TabView`, `TileView`, `Window`, `Menu`, `MsgBox`, `Calendar`, `List`, `Span` |

Each widget follows the same implementation template:
1. `.hpp` public interface in `include/lvgl/widgets/`
2. `.cpp` implementation in `src/widgets/`
3. Unit tests in `tests/widgets/`
4. Example snippet in `docs/design/09-widget-catalog.md`

### Checklist (per widget)
- [ ] Widget creates successfully as child of a `Screen`
- [ ] All setters compile and update internal state
- [ ] All getters return correct values
- [ ] Relevant events fire (`ValueChanged`, `Clicked`, etc.)
- [ ] Widget renders correctly in golden-image test
- [ ] Widget participates in Flex / Grid layout

---

## Phase 6 – Drivers & Integration (Weeks 22–24)

### Deliverables
- `include/lvgl/drivers/` concept definitions + built-in drivers:
  - SDL2 display + mouse/keyboard (`lv::drivers::Sdl2Display`)
  - DRM/KMS display (`lv::drivers::DrmDisplay`)
  - Linux `evdev` input (`lv::drivers::EvdevPointer`, `lv::drivers::EvdevKeypad`)
  - SPI/I2C framebuffer stubs
- Cross-compile integration test on Cortex-M (QEMU)
- `docs/design/` finalized with all cross-references
- Module interface units (`.ixx`) alongside headers
- `meson.build` `install` target: headers, pkg-config file

### Checklist
- [ ] SDL2 demo app compiles and runs a button-press counter
- [ ] DRM demo renders on a Linux framebuffer
- [ ] QEMU ARM Cortex-M integration test passes
- [ ] Module `import lvgl;` compiles under GCC-14 and Clang-17
- [ ] `pkg-config --libs lvgl-cxx` returns correct flags
- [ ] `meson install` places headers and library under prefix
- [ ] All documentation cross-links resolve
- [ ] CodeQL security scan reports zero alerts

---

## 2. Repository Milestones Timeline

```
Week  1   Phase 0 ── Scaffolding
Weeks 2-4 Phase 1 ── Object tree
Weeks 5-7 Phase 2 ── Styles
Weeks 8-9 Phase 3 ── Events / Observer / Groups
Weeks 10-13 Phase 4 ── Display / Renderer / Animations
Weeks 14-21 Phase 5 ── Widget library
Weeks 22-24 Phase 6 ── Drivers + Integration + Modules
```

**Total estimated calendar time: 24 weeks (6 months)**

---

## 3. Testing Strategy

| Test type | Framework | Location |
|---|---|---|
| Unit tests | [Google Test](https://github.com/google/googletest) (system pkg or gtest.wrap) | `tests/unit/` |
| Golden-image tests | Built-in PNG comparator | `tests/golden/` |
| Integration tests | Meson test + QEMU | `tests/integration/` |
| Static analysis | `clang-tidy` + CodeQL | CI only |
| Sanitizers | ASan + UBSan + TSan (GCC/Clang) | CI debug preset |

### Meson test presets

```ini
# meson.options
option('enable_tests',    type: 'boolean', value: true)
option('enable_sanitizers', type: 'boolean', value: false)
option('enable_lto',      type: 'boolean', value: false)
```

---

## 4. CI Matrix

Workflow files live in `.github/workflows/`.

### `ci.yml` — Build & Test

| Runner | Compiler | C++ std | Arch |
|---|---|---|---|
| ubuntu-24.04 | GCC 14 (`gcc-14`) | C++23 | x64 |
| ubuntu-24.04 | Clang 19 (`clang-19`) | C++23 | x64 |
| fedora:latest (container) | GCC (latest) | C++23 | x64 |
| fedora:latest (container) | Clang (latest) | C++23 | x64 |
| macos-15 | Apple Clang 16 (Xcode 16) | C++23 | arm64 |
| macos-15 | LLVM 19 (Homebrew) | C++23 | arm64 |
| macos-13 | LLVM 19 (Homebrew) | C++23 | x64 |
| windows-2022 | MSVC 19.x (`/std:c++latest`) | C++23 | x64 |
| windows-11-arm | MSVC 19.x (`/std:c++latest`) | C++23 | **arm64** |

### `lint.yml` — Code Quality

| Job | Tool | Runner |
|---|---|---|
| clang-format | clang-format-19 (`--dry-run -Werror`) | ubuntu-24.04 |
| clang-tidy | clang-tidy-19 (`--warnings-as-errors='*'`) | ubuntu-24.04 |

### `codeql.yml` — Security Analysis

| Job | Language | Runner | Schedule |
|---|---|---|---|
| analyze | C/C++ (`security-extended,security-and-quality`) | ubuntu-24.04 | push + weekly |

> **Sanitizers** are enabled via `-Denable_sanitizers=true` in the
> `ubuntu-24.04 / clang-19` build (ASan + UBSan).  These are not run on
> macOS or Windows to keep CI times manageable.

---

## 5. Breaking Change Policy

Because lvgl-cxx is explicitly a breaking redesign, it follows these policies:

- **No deprecation warnings** – removed APIs are simply absent.
- **Major version bump** for any change to `Object`'s virtual interface.
- **The C API is never re-exposed** – if C interop is needed, users write a
  thin C shim on top of lvgl-cxx.
- **Semantic versioning** with a changelog entry for every breaking change.
