# SPDX-License-Identifier: MIT
# Copyright (c) 2025 LVGL Kft
# Copyright (c) 2026 Joel Winarske
#
# Tests sub-directory.  Uses Google Test (system package preferred;
# falls back to the gtest.wrap git subproject).

gtest_main_dep = dependency(
  'gtest_main',
  fallback : ['gtest', 'gtest_main_dep'],
  required : true,
)

# Common compiler arguments for test executables
test_cpp_args = []
if meson.get_compiler('cpp').get_id() in ['gcc', 'clang']
  test_cpp_args += ['-Wshadow', '-Wconversion']
endif

# ── Phase 0 — smoke (build / config infrastructure) ──────────────────────────
test(
  'smoke',
  executable(
    'test_smoke',
    'unit/test_smoke.cpp',
    include_directories : [inc],
    dependencies        : [gtest_main_dep, generated_conf_dep],
    cpp_args            : test_cpp_args,
  ),
  protocol : 'gtest',
)

# ── Phase 1 — types (ObjFlags, ObjState, StyleSelector, …) ───────────────────
test(
  'types',
  executable(
    'test_types',
    'unit/test_types.cpp',
    include_directories : [inc],
    dependencies        : [gtest_main_dep, generated_conf_dep],
    cpp_args            : test_cpp_args,
  ),
  protocol : 'gtest',
)

# ── Phase 1 — area (Point + Area) ────────────────────────────────────────────
test(
  'area',
  executable(
    'test_area',
    'unit/test_area.cpp',
    include_directories : [inc],
    dependencies        : [gtest_main_dep, generated_conf_dep],
    cpp_args            : test_cpp_args,
  ),
  protocol : 'gtest',
)

# ── Phase 1 — color (Color, color format tags) ───────────────────────────────
test(
  'color',
  executable(
    'test_color',
    'unit/test_color.cpp',
    include_directories : [inc],
    dependencies        : [gtest_main_dep, generated_conf_dep],
    cpp_args            : test_cpp_args,
  ),
  protocol : 'gtest',
)

# ── Phase 1 — object (Object tree, flags, states, geometry, Screen) ──────────
test(
  'object',
  executable(
    'test_object',
    'unit/test_object.cpp',
    include_directories : [inc],
    dependencies        : [lvgl_cxx_dep, gtest_main_dep],
    cpp_args            : test_cpp_args,
  ),
  protocol : 'gtest',
)

# ── Phase 2 — style (Style, StyleSheet, Transition, Theme) ───────────────────
test(
  'style',
  executable(
    'test_style',
    'unit/test_style.cpp',
    include_directories : [inc],
    dependencies        : [lvgl_cxx_dep, gtest_main_dep],
    cpp_args            : test_cpp_args,
  ),
  protocol : 'gtest',
)

# ── Phase 3 — events, observer, group ────────────────────────────────────────
test(
  'event',
  executable(
    'test_event',
    'unit/test_event.cpp',
    include_directories : [inc],
    dependencies        : [lvgl_cxx_dep, gtest_main_dep],
    cpp_args            : test_cpp_args,
  ),
  protocol : 'gtest',
)

# ── Phase 4 — draw buffer ────────────────────────────────────────────────────
test(
  'draw_buffer',
  executable(
    'test_draw_buffer',
    'unit/test_draw_buffer.cpp',
    include_directories : [inc],
    dependencies        : [gtest_main_dep, generated_conf_dep],
    cpp_args            : test_cpp_args,
  ),
  protocol : 'gtest',
)

# ── Phase 4 — layer ──────────────────────────────────────────────────────────
test(
  'layer',
  executable(
    'test_layer',
    'unit/test_layer.cpp',
    include_directories : [inc],
    dependencies        : [gtest_main_dep, generated_conf_dep],
    cpp_args            : test_cpp_args,
  ),
  protocol : 'gtest',
)

# ── Phase 4 — software renderer ─────────────────────────────────────────────
test(
  'software_renderer',
  executable(
    'test_software_renderer',
    'unit/test_software_renderer.cpp',
    include_directories : [inc],
    dependencies        : [lvgl_cxx_dep, gtest_main_dep],
    cpp_args            : test_cpp_args,
  ),
  protocol : 'gtest',
)

# ── Phase 4 — display ────────────────────────────────────────────────────────
test(
  'display',
  executable(
    'test_display',
    'unit/test_display.cpp',
    include_directories : [inc],
    dependencies        : [lvgl_cxx_dep, gtest_main_dep],
    cpp_args            : test_cpp_args,
  ),
  protocol : 'gtest',
)

# ── Phase 4 — animation and tick ─────────────────────────────────────────────
test(
  'animation',
  executable(
    'test_animation',
    'unit/test_animation.cpp',
    include_directories : [inc],
    dependencies        : [lvgl_cxx_dep, gtest_main_dep],
    cpp_args            : test_cpp_args,
  ),
  protocol : 'gtest',
)

# ── Phase 5 — widgets ────────────────────────────────────────────────────────
test(
  'widgets',
  executable(
    'test_widgets',
    'unit/test_widgets.cpp',
    include_directories : [inc],
    dependencies        : [lvgl_cxx_dep, gtest_main_dep],
    cpp_args            : test_cpp_args,
  ),
  protocol : 'gtest',
)

# ── Font and text rendering ──────────────────────────────────────────────────
test(
  'font',
  executable(
    'test_font',
    'unit/test_font.cpp',
    include_directories : [inc],
    dependencies        : [lvgl_cxx_dep, gtest_main_dep],
    cpp_args            : test_cpp_args,
  ),
  protocol : 'gtest',
)

# ── Phase 6 — drivers, input devices, integration ───────────────────────────
test(
  'drivers',
  executable(
    'test_drivers',
    'unit/test_drivers.cpp',
    include_directories : [inc],
    dependencies        : [lvgl_cxx_dep, gtest_main_dep],
    cpp_args            : test_cpp_args,
  ),
  protocol : 'gtest',
)
