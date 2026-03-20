// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — src/core/observer.cpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// Phase 3: ObserverHandle implementation.
// The Subject<T> template is fully defined inline in observer.hpp; this file
// only provides ObserverHandle's non-template methods.

#include "lvgl/core/observer.hpp"

#include <functional>
#include <memory>

namespace lv {

// ── ObserverHandle::Impl ─────────────────────────────────────────────────────
// Type-erased slot: keep_alive holds the subscriber Entry alive (by owning a
// shared_ptr<void> aliased from the shared_ptr<Entry> created in Subject<T>),
// and deactivate() marks the Entry as inactive so notify() skips it and the
// Subject eventually removes it from its list.

struct ObserverHandle::Impl {
  std::shared_ptr<void>
      keep_alive;  ///< keeps Entry alive as long as handle exists
  std::function<void()>
      deactivate;  ///< marks Entry::active = false; nullptr after release()
};

// ── ObserverHandle
// ────────────────────────────────────────────────────────────

ObserverHandle::~ObserverHandle() {
  // RAII unsubscribe: mark the entry inactive so the Subject skips it on
  // the next notify() call and eventually purges it.
  if (impl_ && impl_->deactivate)
    impl_->deactivate();
}

ObserverHandle::ObserverHandle(ObserverHandle&&) noexcept = default;
ObserverHandle& ObserverHandle::operator=(ObserverHandle&&) noexcept = default;

void ObserverHandle::release() noexcept {
  // Detach from RAII: null out the deactivate callback so the destructor
  // won't unsubscribe.  The Subject keeps the Entry alive.
  if (impl_)
    impl_->deactivate = nullptr;
  impl_.reset();
}

void ObserverHandle::unsubscribe() noexcept {
  if (!impl_)
    return;
  if (impl_->deactivate) {
    impl_->deactivate();
    impl_->deactivate = nullptr;
  }
  impl_.reset();
}

bool ObserverHandle::valid() const noexcept {
  return impl_ != nullptr;
}

// ── Factory (called from Subject<T>::make_handle)
// ─────────────────────────────

ObserverHandle ObserverHandle::from_parts(
    std::shared_ptr<void> keep_alive,
    std::function<void()> deactivate) noexcept {
  ObserverHandle h;
  h.impl_ = std::make_unique<Impl>();
  h.impl_->keep_alive = std::move(keep_alive);
  h.impl_->deactivate = std::move(deactivate);
  return h;
}

}  // namespace lv
