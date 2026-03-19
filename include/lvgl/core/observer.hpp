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

#include "../misc/color.hpp"  // Color — needed for ColorSubject

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
    // Internal factory called by Subject<T>::make_handle (type-erased so the
    // template can produce a handle without needing Impl to be complete here).
    static ObserverHandle from_parts(std::shared_ptr<void>  keep_alive,
                                     std::function<void()>  deactivate) noexcept;

    template <typename T> friend class Subject;

    struct Impl;
    std::unique_ptr<Impl> impl_;
};

// ── Subject<T> ────────────────────────────────────────────────────────────────
// Holds a value; registered observers are notified whenever it changes.
// Mirrors lv_subject_t in LVGL v9.5.0 (src/core/lv_obj_property.c).
template <typename T>
class Subject {
public:
    explicit Subject(T initial = T{})
        : value_(std::move(initial)) {}

    // Non-copyable (each Subject has a unique identity and subscriber list)
    Subject(const Subject&)            = delete;
    Subject& operator=(const Subject&) = delete;
    Subject(Subject&&) noexcept        = default;
    Subject& operator=(Subject&&) noexcept = default;

    // Get / set
    [[nodiscard]] const T& get() const noexcept { return value_; }

    void set(T value) {
        if (value == value_) return;
        value_ = std::move(value);
        notify();
    }

    // Modify in-place (useful for complex types that can't use ==)
    template <typename F>
    void modify(F&& fn) {
        fn(value_);
        notify();
    }

    // Subscribe — fn is called immediately with the current value,
    // then again on every subsequent change.
    // The returned RAII handle auto-unsubscribes on destruction.
    [[nodiscard]] ObserverHandle
    subscribe(std::move_only_function<void(const T&)> fn) {
        fn(value_);  // immediate call with current value
        auto entry = std::make_shared<Entry>(std::move(fn));
        subscribers_.push_back(entry);
        return make_handle(entry);
    }

private:
    struct Entry {
        std::move_only_function<void(const T&)> fn;
        bool active = true;
        explicit Entry(std::move_only_function<void(const T&)> f)
            : fn(std::move(f)) {}
    };

    void notify() {
        for (auto& sp : subscribers_) {
            if (sp && sp->active)
                sp->fn(value_);
        }
        // Purge entries that have been deactivated
        std::erase_if(subscribers_, [](auto& sp) {
            return !sp || !sp->active;
        });
    }

    ObserverHandle make_handle(std::shared_ptr<Entry> entry) {
        return ObserverHandle::from_parts(
            entry,
            [e = entry]() noexcept { e->active = false; });
    }

    T value_;
    // Strong references: the Subject keeps the Entry alive; the handle also
    // holds one.  release() on the handle drops the handle's reference while
    // leaving the Subject's strong reference, keeping the subscription alive.
    std::vector<std::shared_ptr<Entry>> subscribers_;
};

// ── Common specialisations ────────────────────────────────────────────────────
using IntSubject    = Subject<int32_t>;
using FloatSubject  = Subject<float>;
using StringSubject = Subject<std::string>;
using ColorSubject  = Subject<Color>;

}  // namespace lv
