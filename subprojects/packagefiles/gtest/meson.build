# SPDX-License-Identifier: MIT
# Copyright (c) 2026 Joel Winarske
#
# subprojects/packagefiles/gtest/meson.build
#
# Meson port for Google Test v1.14.0.
# This file is overlaid onto the raw googletest git clone via
# gtest.wrap's patch_directory = gtest directive.
#
# Sources reference: https://github.com/google/googletest/tree/v1.14.0

project('gtest', 'cpp',
  version : '1.14.0',
  default_options : ['cpp_std=c++14'],
)

gtest_inc     = include_directories('googletest/include', 'googlemock/include')
gtest_src_inc = include_directories('googletest', 'googlemock')

# ── GoogleTest ────────────────────────────────────────────────────────────────

gtest_lib = static_library('gtest',
  sources : ['googletest/src/gtest-all.cc'],
  include_directories : [gtest_inc, gtest_src_inc],
)

gtest_main_lib = static_library('gtest_main',
  sources : ['googletest/src/gtest_main.cc'],
  include_directories : [gtest_inc, gtest_src_inc],
  link_with : gtest_lib,
)

gtest_dep = declare_dependency(
  include_directories : gtest_inc,
  link_with : gtest_lib,
)

gtest_main_dep = declare_dependency(
  include_directories : gtest_inc,
  link_with : [gtest_lib, gtest_main_lib],
)

# ── GoogleMock ────────────────────────────────────────────────────────────────

gmock_lib = static_library('gmock',
  sources : ['googlemock/src/gmock-all.cc'],
  include_directories : [gtest_inc, gtest_src_inc],
  link_with : gtest_lib,
)

gmock_main_lib = static_library('gmock_main',
  sources : ['googlemock/src/gmock_main.cc'],
  include_directories : [gtest_inc, gtest_src_inc],
  link_with : [gtest_lib, gmock_lib],
)

gmock_dep = declare_dependency(
  include_directories : gtest_inc,
  link_with : [gtest_lib, gmock_lib],
)

gmock_main_dep = declare_dependency(
  include_directories : gtest_inc,
  link_with : [gtest_lib, gmock_lib, gmock_main_lib],
)
