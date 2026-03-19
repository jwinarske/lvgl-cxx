# SPDX-License-Identifier: MIT
# Copyright (c) 2025 LVGL Kft
# Copyright (c) 2026 Joel Winarske
#
# Tests sub-directory.  Populated progressively from Phase 1 onwards.
# Catch2 is fetched via the WrapDB.

catch2_dep = dependency(
  'catch2-with-main',
  fallback : ['catch2', 'catch2_with_main_dep'],
)

# ── Phase 0 smoke test ────────────────────────────────────────────────────────
test(
  'smoke',
  executable(
    'test_smoke',
    'unit/test_smoke.cpp',
    include_directories : [inc],
    dependencies : [lvgl_cxx_dep, catch2_dep, generated_conf_dep],
  ),
)

# Future unit tests are added here as source files are created:
# test('object_tree',  executable('test_object_tree',  'unit/test_object_tree.cpp',  ...))
# test('style',        executable('test_style',        'unit/test_style.cpp',        ...))
# test('event',        executable('test_event',        'unit/test_event.cpp',        ...))
# test('display',      executable('test_display',      'unit/test_display.cpp',      ...))
# test('label',        executable('test_label',        'unit/test_label.cpp',        ...))
# test('button',       executable('test_button',       'unit/test_button.cpp',       ...))
# test('slider',       executable('test_slider',       'unit/test_slider.cpp',       ...))
