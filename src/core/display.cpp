// SPDX-License-Identifier: MIT
// Copyright (c) 2025 LVGL Kft
// Copyright (c) 2026 Joel Winarske
//
// lvgl-cxx — src/core/display.cpp
// Upstream LVGL baseline: v9.5.0
// https://github.com/lvgl/lvgl/releases/tag/v9.5.0
//
// Phase 4: Display implementation — screen management, draw buffers,
// dirty-region tracking, refresh cycle.

#include "lvgl/core/display.hpp"

#include <algorithm>
#include <cassert>
#include <memory>
#include <vector>

#include "lvgl/core/animation.hpp"
#include "lvgl/core/draw_buffer.hpp"
#include "lvgl/core/event.hpp"
#include "lvgl/core/layer.hpp"
#include "lvgl/core/object.hpp"
#include "lvgl/core/screen.hpp"
#include "lvgl/draw/software_renderer.hpp"
#include "lvgl/misc/font.hpp"
#include "lvgl/widgets/arc.hpp"
#include "lvgl/widgets/label.hpp"

#include <cmath>

namespace lv {

// ── Display::Impl ───────────────────────────────────────────────────────────

struct Display::Impl {
  int32_t hor_res{};
  int32_t ver_res{};
  DisplayRotation rotation = DisplayRotation::None;
  RenderMode render_mode = RenderMode::Partial;
  bool antialiasing = false;
  uint8_t backlight = 255;

  FlushFn flush_fn;

  // Screens owned by this display
  std::vector<std::unique_ptr<Screen>> screens;
  Screen* active_screen = nullptr;

  // Draw buffers (user-provided spans, not owned)
  Pixel* buf1_data = nullptr;
  std::size_t buf1_size = 0;
  Pixel* buf2_data = nullptr;
  std::size_t buf2_size = 0;
  int active_buf = 0;  // 0 or 1 for double-buffering

  // Dirty region tracking — list of invalidated areas
  std::vector<Area> dirty_areas;
  static constexpr int MaxDirtyAreas = 32;

  // Event handlers on the display itself
  struct HandlerEntry {
    EventCode code;
    UniqueFunction<void(Event&)> fn;
    uint64_t id;
    bool active = true;
  };
  std::vector<HandlerEntry> handlers;
  uint64_t next_handler_id = 0;

  // Renderer
  SoftwareRenderer renderer;

  void add_dirty(Area a) {
    // Clamp to screen bounds
    a.x1 = std::max(a.x1, int32_t{0});
    a.y1 = std::max(a.y1, int32_t{0});
    a.x2 = std::min(a.x2, hor_res - 1);
    a.y2 = std::min(a.y2, ver_res - 1);
    if (a.empty())
      return;

    // Try to coalesce with existing dirty area
    for (auto& da : dirty_areas) {
      if (da.intersects(a)) {
        da = da.unite(a);
        return;
      }
    }

    if (static_cast<int>(dirty_areas.size()) >= MaxDirtyAreas) {
      // Merge all into one big area
      Area merged = dirty_areas[0];
      for (std::size_t i = 1; i < dirty_areas.size(); ++i)
        merged = merged.unite(dirty_areas[i]);
      merged = merged.unite(a);
      dirty_areas.clear();
      dirty_areas.push_back(merged);
    } else {
      dirty_areas.push_back(a);
    }
  }
};

// ── Constructor / Destructor ────────────────────────────────────────────────

Display::Display(int32_t w, int32_t h, FlushFn flush_fn)
    : impl_(std::make_unique<Impl>()) {
  impl_->hor_res = w;
  impl_->ver_res = h;
  impl_->flush_fn = std::move(flush_fn);

  // Create default screen sized to the display resolution
  auto scr = std::make_unique<Screen>(this);
  scr->set_size(w, h);
  impl_->active_screen = scr.get();
  impl_->screens.push_back(std::move(scr));
}

Display::~Display() = default;

// ── Screens ─────────────────────────────────────────────────────────────────

Screen& Display::active_screen() noexcept {
  return *impl_->active_screen;
}

const Screen& Display::active_screen() const noexcept {
  return *impl_->active_screen;
}

Screen& Display::load_screen(std::unique_ptr<Screen> scr,
                             ScreenLoadAnim /*anim*/,
                             uint32_t /*duration*/,
                             uint32_t /*delay*/) {
  Screen& ref = *scr;
  impl_->active_screen = &ref;
  impl_->screens.push_back(std::move(scr));
  // Mark entire screen as dirty for redraw
  impl_->add_dirty(Area::from_size(0, 0, impl_->hor_res, impl_->ver_res));
  return ref;
}

Screen& Display::create_screen() {
  auto scr = std::make_unique<Screen>(this);
  scr->set_size(impl_->hor_res, impl_->ver_res);
  Screen& ref = *scr;
  impl_->screens.push_back(std::move(scr));
  return ref;
}

// ── Geometry ────────────────────────────────────────────────────────────────

int32_t Display::horizontal_resolution() const noexcept {
  return impl_->hor_res;
}

int32_t Display::vertical_resolution() const noexcept {
  return impl_->ver_res;
}

void Display::set_resolution(int32_t w, int32_t h) {
  impl_->hor_res = w;
  impl_->ver_res = h;
  if (impl_->active_screen)
    impl_->active_screen->set_size(w, h);
  impl_->add_dirty(Area::from_size(0, 0, w, h));
}

void Display::set_rotation(DisplayRotation r) {
  impl_->rotation = r;
  impl_->add_dirty(Area::from_size(0, 0, impl_->hor_res, impl_->ver_res));
}

DisplayRotation Display::rotation() const noexcept {
  return impl_->rotation;
}

// ── Draw buffers ────────────────────────────────────────────────────────────

void Display::set_draw_buffers(std::span<Pixel> buf1, std::span<Pixel> buf2) {
  impl_->buf1_data = buf1.data();
  impl_->buf1_size = buf1.size();
  impl_->buf2_data = buf2.empty() ? nullptr : buf2.data();
  impl_->buf2_size = buf2.size();
  impl_->active_buf = 0;
}

// ── Render ──────────────────────────────────────────────────────────────────

void Display::refresh() {
  // Process animations
  anim_tick();

  if (impl_->dirty_areas.empty() && impl_->render_mode != RenderMode::Full) {
    return;
  }

  // If full mode, mark everything dirty
  if (impl_->render_mode == RenderMode::Full) {
    impl_->dirty_areas.clear();
    impl_->dirty_areas.push_back(
        Area::from_size(0, 0, impl_->hor_res, impl_->ver_res));
  }

  // Select buffer
  Pixel* buf_data =
      (impl_->active_buf == 0) ? impl_->buf1_data : impl_->buf2_data;
  if (!buf_data)
    buf_data = impl_->buf1_data;
  if (!buf_data)
    return;  // No buffer set

  const auto buf_size =
      (impl_->active_buf == 0) ? impl_->buf1_size : impl_->buf2_size;
  if (buf_size == 0)
    return;

  // Process each dirty area
  for (const auto& dirty : impl_->dirty_areas) {
    const int32_t area_w = dirty.width();
    const int32_t area_h = dirty.height();
    const auto area_pixels =
        static_cast<std::size_t>(area_w) * static_cast<std::size_t>(area_h);

    // If buffer can hold the entire dirty area, render in one pass
    if (area_pixels <= buf_size) {
      DrawBuffer<Pixel> draw_buf(buf_data, area_w, area_h);

      // Clear buffer to opaque white (screen background)
      draw_buf.fill(detail::color_to_pixel<Pixel>(Color::White()));

      // Collect draw tasks from the active screen's object tree
      Layer layer(dirty);
      collect_draw_tasks(*impl_->active_screen, layer, dirty);

      // Render
      impl_->renderer.execute(draw_buf, layer.tasks());

      // Flush to hardware
      if (impl_->flush_fn) {
        BufferView2D<Pixel> view{buf_data, area_h, area_w};
        impl_->flush_fn(*this, dirty, view);
      }
    } else {
      // Split into horizontal bands
      const auto rows_per_band = std::max(
          int32_t{1},
          static_cast<int32_t>(buf_size / static_cast<std::size_t>(area_w)));

      for (int32_t row = dirty.y1; row <= dirty.y2; row += rows_per_band) {
        const int32_t band_y2 = std::min(row + rows_per_band - 1, dirty.y2);
        const int32_t band_h = band_y2 - row + 1;

        Area band_area = {dirty.x1, row, dirty.x2, band_y2};

        DrawBuffer<Pixel> draw_buf(buf_data, area_w, band_h);
        draw_buf.fill(detail::color_to_pixel<Pixel>(Color::White()));

        Layer layer(band_area);
        collect_draw_tasks(*impl_->active_screen, layer, band_area);

        impl_->renderer.execute(draw_buf, layer.tasks());

        if (impl_->flush_fn) {
          BufferView2D<Pixel> view{buf_data, band_h, area_w};
          impl_->flush_fn(*this, band_area, view);
        }
      }
    }
  }

  impl_->dirty_areas.clear();

  // Swap double-buffer
  if (impl_->buf2_data)
    impl_->active_buf = 1 - impl_->active_buf;
}

void Display::set_render_mode(RenderMode m) {
  impl_->render_mode = m;
}

void Display::set_antialiasing(bool en) {
  impl_->antialiasing = en;
}

// ── Backlight ───────────────────────────────────────────────────────────────

void Display::set_backlight(uint8_t level) {
  impl_->backlight = level;
}

// ── Events ──────────────────────────────────────────────────────────────────

EventHandle Display::on(EventCode code, UniqueFunction<void(Event&)> handler) {
  const uint64_t id = ++impl_->next_handler_id;
  impl_->handlers.push_back({code, std::move(handler), id, true});

  // Return an empty (valid-but-detached) handle
  EventHandle h;
  return h;
}

// ── Static helper: collect draw tasks from object tree ──────────────────────

void Display::collect_draw_tasks(Object& obj,
                                 Layer& layer,
                                 const Area& clip,
                                 Point origin) {
  // Skip hidden objects
  if (obj.has_flag(ObjFlags::Hidden))
    return;

  // Compute absolute screen-space bounds by adding the accumulated origin
  // to the object's parent-relative position.
  const Area local = obj.bounds();
  const Area abs_bounds = {
      local.x1 + origin.x,
      local.y1 + origin.y,
      local.x2 + origin.x,
      local.y2 + origin.y,
  };

  if (!abs_bounds.intersects(clip))
    return;

  // Draw this object's background using resolved styles
  auto bg_color_opt = obj.resolve_style(prop::BgColor{}, obj.state());
  if (bg_color_opt) {
    RectDescriptor dsc;
    dsc.bg_color = *bg_color_opt;
    auto opa = obj.resolve_style(prop::BgOpacity{}, obj.state());
    dsc.bg_opa = opa ? static_cast<uint8_t>(*opa) : uint8_t{255};
    auto rad = obj.resolve_style(prop::Radius{}, obj.state());
    dsc.radius = rad.value_or(0);

    auto bc = obj.resolve_style(prop::BorderColor{}, obj.state());
    if (bc)
      dsc.border_color = *bc;
    auto bw = obj.resolve_style(prop::BorderWidth{}, obj.state());
    dsc.border_width = bw.value_or(0);

    layer.draw_rect(abs_bounds, dsc);
  }

  // If this object is a Label, emit a LabelTask for text rendering
  if (auto* label = dynamic_cast<Label*>(&obj)) {
    auto txt = label->text();
    if (!txt.empty()) {
      LabelDescriptor ldsc;
      ldsc.text = txt;
      auto tc = obj.resolve_style(prop::TextColor{}, obj.state());
      ldsc.color = tc.value_or(Color::Black());
      auto fp = obj.resolve_style(prop::TextFont{}, obj.state());
      ldsc.font = fp.value_or(&font_default());
      auto ls = obj.resolve_style(prop::TextLetterSpacing{}, obj.state());
      ldsc.letter_spacing = ls.value_or(0);
      auto lsp = obj.resolve_style(prop::TextLineSpacing{}, obj.state());
      ldsc.line_spacing = lsp.value_or(0);

      layer.draw_label({abs_bounds.x1, abs_bounds.y1}, ldsc);
    }
  }

  // If this object is an Arc, emit an ArcTask
  if (auto* arc = dynamic_cast<Arc*>(&obj)) {
    ArcDescriptor adsc;
    adsc.color = Color::from_hex(0x009688);  // teal default
    auto ac = obj.resolve_style(prop::BgColor{}, obj.state());
    if (ac)
      adsc.color = *ac;
    adsc.opa = 255;
    adsc.radius = std::min(abs_bounds.width(), abs_bounds.height()) / 2;
    adsc.width = std::max(int32_t{3}, adsc.radius / 4);
    adsc.start_angle = arc->bg_angle_start() + arc->rotation();
    adsc.end_angle = arc->bg_angle_end() + arc->rotation();
    // Map value to foreground angle range
    const int32_t range = arc->max_value() - arc->min_value();
    if (range > 0) {
      const int32_t span = adsc.end_angle - adsc.start_angle;
      adsc.end_angle =
          adsc.start_angle + span * (arc->value() - arc->min_value()) / range;
    }

    Point center = {(abs_bounds.x1 + abs_bounds.x2) / 2,
                    (abs_bounds.y1 + abs_bounds.y2) / 2};
    layer.draw_arc(center, adsc);
  }

  // Children are positioned relative to this object's absolute origin
  const Point child_origin = {abs_bounds.x1, abs_bounds.y1};
  for (auto& child : obj.children()) {
    collect_draw_tasks(*child, layer, clip, child_origin);
  }
}

}  // namespace lv
