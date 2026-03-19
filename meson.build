# SPDX-License-Identifier: MIT
# Copyright (c) 2025 LVGL Kft
# Copyright (c) 2026 Joel Winarske

project(
  'lvgl-cxx',
  'cpp',
  version : '1.0.0-dev',
  license : 'MIT',
  meson_version : '>= 1.3.0',
  default_options : [
    'cpp_std=c++23',
    'warning_level=3',
    'b_ndebug=if-release',
  ],
)

# ── Compiler sanity check ────────────────────────────────────────────────────
cxx = meson.get_compiler('cpp')
if cxx.get_id() == 'gcc' and cxx.version().version_compare('< 14')
  error('GCC >= 14 required for C++23 support')
endif
if cxx.get_id() == 'clang' and cxx.version().version_compare('< 17')
  error('Clang >= 17 required for C++23 support')
endif

# ── User options (see meson.options) ────────────────────────────────────────
opt_color_fmt    = get_option('color_format')
opt_flex         = get_option('use_flex')
opt_grid         = get_option('use_grid')
opt_anim         = get_option('use_animations')
opt_observer     = get_option('use_observer')
opt_shadow       = get_option('use_shadow')
opt_outline      = get_option('use_outline')
opt_layer_xform  = get_option('use_layer_transform')
opt_scroll_anim  = get_option('use_scroll_anim')
opt_max_disp     = get_option('max_displays')
opt_max_indev    = get_option('max_indev_devices')
opt_max_anim     = get_option('max_animations')
opt_dpi          = get_option('default_dpi')
opt_tests        = get_option('enable_tests')
opt_sanitizers   = get_option('enable_sanitizers')
opt_lto          = get_option('enable_lto')
opt_modules      = get_option('enable_modules')

# ── LVGL v9.5.0 upstream dependency (C reference, optional) ─────────────────
# The upstream LVGL C library is declared as a subproject so the software
# renderer implementation can reference LVGL v9.5.0 source directly during
# the porting phase.  It is not exposed in the lvgl-cxx public API.
lvgl_dep = dependency('lvgl',
  fallback : ['lvgl', 'lvgl_dep'],
  required : false,
)

# ── Generated configuration header ──────────────────────────────────────────
conf_data = configuration_data()
conf_data.set('LVGLCXX_VERSION_MAJOR', 1)
conf_data.set('LVGLCXX_VERSION_MINOR', 0)
conf_data.set('LVGLCXX_VERSION_PATCH', 0)
conf_data.set_quoted('LVGLCXX_LVGL_BASELINE', 'v9.5.0')
conf_data.set_quoted('LVGLCXX_LVGL_BASELINE_URL',
  'https://github.com/lvgl/lvgl/releases/tag/v9.5.0')

conf_data.set('LVGLCXX_COLOR_ARGB8888', opt_color_fmt == 'argb8888' ? 1 : 0)
conf_data.set('LVGLCXX_COLOR_RGB888',   opt_color_fmt == 'rgb888'   ? 1 : 0)
conf_data.set('LVGLCXX_COLOR_RGB565',   opt_color_fmt == 'rgb565'   ? 1 : 0)
conf_data.set('LVGLCXX_COLOR_L8',       opt_color_fmt == 'l8'       ? 1 : 0)

conf_data.set('LVGLCXX_USE_FLEX',            opt_flex        ? 1 : 0)
conf_data.set('LVGLCXX_USE_GRID',            opt_grid        ? 1 : 0)
conf_data.set('LVGLCXX_USE_ANIMATIONS',      opt_anim        ? 1 : 0)
conf_data.set('LVGLCXX_USE_OBSERVER',        opt_observer    ? 1 : 0)
conf_data.set('LVGLCXX_USE_SHADOW',          opt_shadow      ? 1 : 0)
conf_data.set('LVGLCXX_USE_OUTLINE',         opt_outline     ? 1 : 0)
conf_data.set('LVGLCXX_USE_LAYER_TRANSFORM', opt_layer_xform ? 1 : 0)
conf_data.set('LVGLCXX_USE_SCROLL_ANIM',     opt_scroll_anim ? 1 : 0)

conf_data.set('LVGLCXX_MAX_DISPLAYS',        opt_max_disp)
conf_data.set('LVGLCXX_MAX_INDEV_DEVICES',   opt_max_indev)
conf_data.set('LVGLCXX_MAX_ANIMATIONS',      opt_max_anim)
conf_data.set('LVGLCXX_DEFAULT_DPI',         opt_dpi)

generated_conf = configure_file(
  input  : 'include/lvgl/lvgl_config.hpp.in',
  output : 'lvgl_config.hpp',
  configuration : conf_data,
)
generated_conf_dep = declare_dependency(
  sources : generated_conf,
  include_directories : include_directories('.'),
)

# ── Include directories ──────────────────────────────────────────────────────
inc = include_directories('include')

# ── Compiler flags ───────────────────────────────────────────────────────────
extra_args = []
if cxx.get_id() in ['gcc', 'clang']
  extra_args += ['-Wshadow', '-Wconversion', '-Wsign-conversion']
endif

if opt_sanitizers and not meson.is_cross_build()
  extra_args += ['-fsanitize=address,undefined', '-fno-omit-frame-pointer']
  add_project_link_arguments('-fsanitize=address,undefined', language : 'cpp')
endif

if opt_lto
  extra_args += ['-flto']
  add_project_link_arguments('-flto', language : 'cpp')
endif

# ── Library sources ──────────────────────────────────────────────────────────
lvgl_cxx_sources = files(
  # Phase 1 – Core object tree
  'src/core/object.cpp',
  'src/core/screen.cpp',
  'src/core/event.cpp',
  # Phase 2 – Styles
  'src/core/style.cpp',
  'src/core/theme.cpp',
  # Phase 3 – Events / Observer / Group
  'src/core/observer.cpp',
  'src/core/group.cpp',
  # Phase 4 – Display / Renderer / Animations
  # 'src/draw/sw/software_renderer.cpp',
  # 'src/core/display.cpp',
  # Phase 5 – Widgets (one file per widget)
)

# ── Module interface units (Phase 6, GCC/Clang modules support) ──────────────
lvgl_cxx_module_sources = []
if opt_modules
  # Populated in Phase 6 when module interface units (.ixx) are authored
  # lvgl_cxx_module_sources += files('src/lvgl.ixx')
endif

# ── Static library target ────────────────────────────────────────────────────
lvgl_cxx_lib = static_library(
  'lvgl-cxx',
  sources : lvgl_cxx_sources,
  include_directories : inc,
  cpp_args : extra_args,
  dependencies : [generated_conf_dep, lvgl_dep],
  install : true,
)

lvgl_cxx_dep = declare_dependency(
  link_with : lvgl_cxx_lib,
  include_directories : inc,
  dependencies : [generated_conf_dep],
)

# ── pkg-config ───────────────────────────────────────────────────────────────
pkg = import('pkgconfig')
pkg.generate(
  lvgl_cxx_lib,
  name        : 'lvgl-cxx',
  description : 'LVGL native C++23 redesign (baseline: LVGL v9.5.0)',
  url         : 'https://github.com/jwinarske/lvgl-cxx',
  version     : meson.project_version(),
)

# ── Install headers ───────────────────────────────────────────────────────────
install_headers(
  'include/lvgl/lvgl.hpp',
  subdir : 'lvgl',
)
install_subdir('include/lvgl/core',    install_dir : 'include/lvgl')
install_subdir('include/lvgl/misc',    install_dir : 'include/lvgl')

# ── Tests ─────────────────────────────────────────────────────────────────────
if opt_tests
  subdir('tests')
endif

# ── Summary ──────────────────────────────────────────────────────────────────
summary({
  'Version'         : meson.project_version(),
  'LVGL baseline'   : 'v9.5.0',
  'Color format'    : opt_color_fmt,
  'Flex layout'     : opt_flex,
  'Grid layout'     : opt_grid,
  'Animations'      : opt_anim,
  'Observer'        : opt_observer,
  'Shadow'          : opt_shadow,
  'Modules (C++23)' : opt_modules,
  'Tests'           : opt_tests,
  'Sanitizers'      : opt_sanitizers,
  'LTO'             : opt_lto,
}, section : 'lvgl-cxx configuration')
