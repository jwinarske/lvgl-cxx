# lvgl-cxx — LVGL Native C++23 Redesign

A **breaking-change, ground-up native C++23 redesign** of the
[LVGL](https://github.com/lvgl/lvgl) embedded GUI library.

This project discards the `extern "C"` compatibility shim and rewrites the
entire API surface using C++23 language features: modules, concepts,
`std::expected`, `std::move_only_function`, deducing-`this`, structured
bindings, `std::mdspan`, `std::generator`, and `std::print`.

## License

MIT — see [LICENCE.txt](LICENCE.txt).  
Original LVGL Copyright © 2025 LVGL Kft.  
lvgl-cxx Copyright © 2026 Joel Winarske.  
Based on [LVGL v9.5.0](https://github.com/lvgl/lvgl/releases/tag/v9.5.0).

## Documentation

| Document | Description |
|---|---|
| [docs/design/01-overview.md](docs/design/01-overview.md) | Goals, non-goals, design principles |
| [docs/design/02-architecture.md](docs/design/02-architecture.md) | High-level module map and dependency graph |
| [docs/design/03-object-model.md](docs/design/03-object-model.md) | Widget class hierarchy, ownership, tree |
| [docs/design/04-style-system.md](docs/design/04-style-system.md) | Property descriptors, cascading, themes |
| [docs/design/05-event-system.md](docs/design/05-event-system.md) | Signal/slot, type-safe event dispatch |
| [docs/design/06-rendering-pipeline.md](docs/design/06-rendering-pipeline.md) | Draw tasks, layers, draw buffers |
| [docs/design/07-memory-model.md](docs/design/07-memory-model.md) | Allocator concepts, arena, smart pointers |
| [docs/design/08-configuration.md](docs/design/08-configuration.md) | Compile-time policy types, feature flags |
| [docs/design/09-widget-catalog.md](docs/design/09-widget-catalog.md) | All 30+ widget API sketches |
| [docs/design/10-porting-guide.md](docs/design/10-porting-guide.md) | C → C++23 migration reference |
| [docs/design/11-implementation-plan.md](docs/design/11-implementation-plan.md) | Phased roadmap with milestones |

## Repository Layout

```
lvgl-cxx/
├── include/lvgl/          # Public C++23 headers / module interface units
│   ├── lvgl.hpp           # Umbrella include
│   ├── core/              # Object, event, style, display, animation, group
│   ├── widgets/           # All widget classes
│   ├── misc/              # Color, area, font, timer, math utilities
│   └── drivers/           # Display & input-device driver abstractions
├── src/                   # Implementation units (added during implementation phases)
├── docs/design/           # Technical design documentation
├── tests/                 # Unit tests (added during implementation phases)
├── meson.build            # Top-level Meson build definition
├── meson.options          # User-visible build options
└── LICENCE.txt
```

## Compiler Requirements

| Compiler | Minimum version | C++23 flag |
|---|---|---|
| GCC | 14 | `-std=c++23` |
| Clang | 17 | `-std=c++23` |
| MSVC | 19.38 (VS 2022 17.8) | `/std:c++latest` |
| ARM Clang (LLVM) | 17 | `-std=c++23` |

## CI Status

Workflows are defined in `.github/workflows/`:

| Workflow | Jobs |
|---|---|
| `ci.yml` | Ubuntu 24.04 (GCC 14, Clang 19), Fedora latest (GCC, Clang), macOS 15 arm64 (Apple Clang 16, LLVM 19), macOS 13 x64 (LLVM 19), Windows x64 (MSVC), **Windows arm64 (MSVC)** |
| `lint.yml` | clang-format-19 (format check), clang-tidy-19 (static analysis) |
| `codeql.yml` | CodeQL C/C++ security analysis (push + weekly schedule) |

Lint and format tooling from [jwinarske/wayland-cxx-scanner](https://github.com/jwinarske/wayland-cxx-scanner).

```sh
meson setup build -Dcpp_std=c++23
meson compile -C build
meson test -C build          # run unit tests
```

Cross-compile for ARM Cortex-M:

```sh
meson setup build --cross-file cross/arm-none-eabi.ini
meson compile -C build
```

## Quick API Preview

```cpp
import lvgl;

int main() {
    lv::Display display{480, 320};

    auto& screen = display.active_screen();

    auto& btn = screen.create<lv::Button>();
    btn.set_size(120, 50)
       .align(lv::Align::Center)
       .on(lv::Event::Clicked, [](lv::Event& e) {
           lv::print("Button clicked!\n");
       });

    auto& label = btn.create<lv::Label>();
    label.set_text("Hello C++23")
         .align(lv::Align::Center);

    lv::run_loop();
}
```
