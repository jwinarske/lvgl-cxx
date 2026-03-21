// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — misc/function.hpp
//
// Portability wrapper for move-only callable types.
//
// std::move_only_function (C++23) is not available in all toolchain/stdlib
// combinations that the project supports — notably Apple Clang 16 and
// Homebrew LLVM 19 on macOS where libc++ gating restricts it regardless of
// the -std=c++23 flag.
//
// lv::UniqueFunction<Sig> selects the best available option:
//   • std::move_only_function<Sig>  — preferred (move-only, no copy overhead)
//   • std::function<Sig>            — fallback  (requires copyable callables)

#pragma once

#include <functional>
#include <version>  // feature-test macros

namespace lv {

#if defined(__cpp_lib_move_only_function) && \
    __cpp_lib_move_only_function >= 202110L
template <typename Sig>
using UniqueFunction = std::move_only_function<Sig>;
#else
template <typename Sig>
using UniqueFunction = std::function<Sig>;
#endif

}  // namespace lv
