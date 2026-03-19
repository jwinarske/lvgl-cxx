// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — core/observer.hpp
// Upstream LVGL baseline: v9.5.0  https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// Type-safe observer / data-binding.  Replaces lv_observer.h.

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace lv {

class Object;

// ── ObserverHandle ────────────────────────────────────────────────────────────
// RAII subscription token.  Destroying the handle unsubscribes automatically.
// Call release() to detach from RAII (subscription lives until Subject destroyed).
class [[nodiscard]] ObserverHandle {
public:
    ObserverHandle() noexcept = default;
    ~ObserverHandle();

    ObserverHandle(ObserverHandle&&) noexcept;
    ObserverHandle& operator=(ObserverHandle&&) noexcept;

    void release() noexcept;
    void unsubscribe() noexcept;
    [[nodiscard]] bool valid() const noexcept;
    explicit operator bool() const noexcept { return valid(); }

private:
    friend class SubjectBase;
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

// ── Subject<T> ────────────────────────────────────────────────────────────────
template<typename T>
class Subject {
public:
    explicit Subject(T initial = T{})
        : value_(std::move(initial)) {}

    // Get / set
    [[nodiscard]] const T& get() const noexcept { return value_; }

    void set(T value) {
        if (value == value_) return;
        value_ = std::move(value);
        notify();
    }

    // Modify in-place (useful for complex types)
    template<typename F>
    void modify(F&& fn) {
        fn(value_);
        notify();
    }

    // Subscribe — fn is called immediately with current value, then on each change.
    [[nodiscard]] ObserverHandle subscribe(
            std::move_only_function<void(const T&)> fn) {
        fn(value_);                              // immediate call
        auto entry = std::make_shared<Entry>(std::move(fn));
        subscribers_.push_back(entry);
        return make_handle(entry);
    }

    // Convenience: bind subject value to obj.set_text() (requires std::format)
    // Returns RAII handle — destroy to unbind.
    [[nodiscard]] ObserverHandle bind_label_text(Object& obj);

private:
    struct Entry {
        std::move_only_function<void(const T&)> fn;
        bool active = true;
        explicit Entry(std::move_only_function<void(const T&)> f)
            : fn(std::move(f)) {}
    };

    void notify() {
        for (auto& wp : subscribers_) {
            if (auto sp = wp.lock(); sp && sp->active)
                sp->fn(value_);
        }
        // Purge expired weak_ptrs
        std::erase_if(subscribers_, [](auto& wp){ return wp.expired(); });
    }

    ObserverHandle make_handle(std::shared_ptr<Entry> entry);

    T value_;
    std::vector<std::weak_ptr<Entry>> subscribers_;
};

// ── Common specialisations ────────────────────────────────────────────────────
using IntSubject    = Subject<int32_t>;
using FloatSubject  = Subject<float>;
using StringSubject = Subject<std::string>;
using ColorSubject  = Subject<Color>;   // Color from misc/color.hpp

} // namespace lv

#include "../misc/color.hpp"   // needed for ColorSubject
