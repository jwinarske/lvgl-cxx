# 06 – Rendering Pipeline

## 1. C → C++23 Mapping

| C (LVGL v9.5.0) | C++23 (lvgl-cxx) |
|---|---|
| `lv_display_t` struct | `lv::Display` class |
| `lv_display_set_flush_cb` | Constructor / policy template parameter |
| `lv_draw_buf_t` | `lv::DrawBuffer` (owns `std::mdspan<Pixel,2>`) |
| `lv_draw_task_t` | `lv::DrawTask` variant |
| `lv_layer_t` | `lv::Layer` |
| `lv_draw_rect_dsc_t` | `lv::RectDescriptor` |
| `lv_draw_label_dsc_t` | `lv::LabelDescriptor` |
| `lv_draw_image_dsc_t` | `lv::ImageDescriptor` |
| `lv_draw_line_dsc_t` | `lv::LineDescriptor` |
| `lv_draw_arc_dsc_t`  | `lv::ArcDescriptor` |
| `lv_draw_triangle_dsc_t` | `lv::TriangleDescriptor` |
| Software renderer | `lv::SoftwareRenderer` (concept-satisfying class) |
| `lv_refr_now` | `display.refresh()` |
| `lv_obj_invalidate` | `obj.invalidate()` |

## 2. Display Class

```cpp
namespace lv {

// Flush callback concept – what the user implements to push pixels to hardware
template<typename F, typename Pixel>
concept FlushCallback = requires(F fn,
                                 Display& disp,
                                 const Area& area,
                                 std::mdspan<const Pixel, std::dextents<int,2>> buf) {
    { fn(disp, area, buf) } -> std::same_as<void>;
};

// Display owns screens, a draw buffer, and a renderer instance.
template<typename Config = DefaultConfig>
class Display {
public:
    using Pixel    = typename Config::ColorFormat::pixel_type;
    using Renderer = typename Config::Renderer;

    // Construct with resolution and a flush callback
    template<FlushCallback<Pixel> F>
    Display(int32_t w, int32_t h, F&& flush_cb);

    ~Display();

    // Screens
    [[nodiscard]] Screen&       active_screen()       noexcept;
    [[nodiscard]] const Screen& active_screen() const noexcept;

    Screen& load_screen(std::unique_ptr<Screen> scr,
                        ScreenLoadAnim anim = ScreenLoadAnim::None,
                        uint32_t duration_ms = 0,
                        uint32_t delay_ms    = 0);

    Screen& create_screen();

    // Geometry
    [[nodiscard]] int32_t horizontal_resolution() const noexcept;
    [[nodiscard]] int32_t vertical_resolution()   const noexcept;
    void set_resolution(int32_t w, int32_t h);
    void set_rotation(DisplayRotation r);

    // Draw buffer management
    void set_draw_buffers(std::span<Pixel> buf1,
                          std::span<Pixel> buf2 = {});  // double-buffer

    // Render
    void refresh();                     // manual trigger
    void set_render_mode(RenderMode m); // full / partial / direct
    void set_antialiasing(bool en);

    // Backlight / brightness (optional – no-op if driver doesn't support)
    void set_backlight(uint8_t level_0_to_255);

    // Events on the display itself
    EventHandle on(EventCode code,
                   std::move_only_function<void(Event&)> handler);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace lv
```

## 3. Draw Buffer

`DrawBuffer` wraps a `std::mdspan<Pixel, std::dextents<int,2>>` over a
user-provided or library-allocated memory region:

```cpp
namespace lv {

template<typename Pixel>
class DrawBuffer {
public:
    using view_type       = std::mdspan<Pixel,       std::dextents<int,2>>;
    using const_view_type = std::mdspan<const Pixel, std::dextents<int,2>>;

    // Non-owning view over caller-managed storage
    DrawBuffer(Pixel* data, int32_t w, int32_t h) noexcept;

    // Owning, allocator-aware buffer
    template<typename Alloc = std::allocator<Pixel>>
    static DrawBuffer allocate(int32_t w, int32_t h,
                               Alloc alloc = {});

    [[nodiscard]] view_type       view()       noexcept;
    [[nodiscard]] const_view_type view() const noexcept;

    // Pixel access  (C++23 multidimensional subscript)
    Pixel&       operator[](int32_t row, int32_t col)       noexcept;
    const Pixel& operator[](int32_t row, int32_t col) const noexcept;

    [[nodiscard]] int32_t width()  const noexcept;
    [[nodiscard]] int32_t height() const noexcept;
};

} // namespace lv
```

## 4. Draw Tasks

Each `Object::on_draw()` implementation pushes typed draw tasks into a
`Layer`.  The renderer then processes the task queue.

```cpp
namespace lv {

// All draw task variants share a base
struct DrawTaskBase {
    Area     clip_area;
    int32_t  layer_id = 0;
};

struct RectTask      : DrawTaskBase { Area bounds; RectDescriptor  dsc; };
struct LabelTask     : DrawTaskBase { Point pos;   LabelDescriptor dsc; };
struct ImageTask     : DrawTaskBase { Area bounds; ImageDescriptor dsc; };
struct LineTask      : DrawTaskBase { Point p1, p2; LineDescriptor dsc; };
struct ArcTask       : DrawTaskBase { Point center; ArcDescriptor  dsc; };
struct TriangleTask  : DrawTaskBase { std::array<Point,3> pts; TriangleDescriptor dsc; };
struct VectorTask    : DrawTaskBase { /* SVG-style vector path */ };
struct MaskTask      : DrawTaskBase { /* clipping mask */ };

using DrawTask = std::variant<
    RectTask, LabelTask, ImageTask, LineTask,
    ArcTask,  TriangleTask, VectorTask, MaskTask
>;

} // namespace lv
```

## 5. Renderer Concept

```cpp
namespace lv {

template<typename R, typename Pixel>
concept Renderer = requires(R renderer,
                            DrawBuffer<Pixel>& buf,
                            std::span<const DrawTask> tasks) {
    { renderer.execute(buf, tasks) } -> std::same_as<void>;
};

// Built-in software renderer satisfies Renderer<SoftwareRenderer, ARGB8888::pixel_type>
class SoftwareRenderer {
public:
    template<typename Pixel>
    void execute(DrawBuffer<Pixel>& buf, std::span<const DrawTask> tasks);
};
static_assert(Renderer<SoftwareRenderer, color::ARGB8888::pixel_type>);

} // namespace lv
```

## 6. Layer

A `Layer` is an intermediate render target (e.g. for opacity groups,
transformations, or snapshot widgets):

```cpp
namespace lv {

class Layer {
public:
    void draw_rect    (Area bounds,        RectDescriptor   dsc);
    void draw_label   (Point pos,          LabelDescriptor  dsc);
    void draw_image   (Area bounds,        ImageDescriptor  dsc);
    void draw_line    (Point p1, Point p2, LineDescriptor   dsc);
    void draw_arc     (Point center,       ArcDescriptor    dsc);
    void draw_triangle(std::array<Point,3> pts, TriangleDescriptor dsc);
    void draw_vector  (/* vector path */);

    void set_clip_area(Area a) noexcept;
    [[nodiscard]] Area clip_area() const noexcept;
};

} // namespace lv
```

## 7. Invalidation and Partial Refresh

Dirty-region tracking uses a run-length encoded region list (same algorithm
as upstream LVGL) computed at the `Display` level:

```
Object::invalidate()
  └─► marks Area as dirty on owning Screen
        └─► Screen coalesces dirty areas
              └─► Display::refresh() clips & sorts draw tasks
                    └─► Renderer::execute()
                          └─► flush_cb() → hardware
```

Partial refresh avoids re-drawing unchanged pixels, critical for low-bandwidth
SPI-connected displays.

## 8. Snapshot

```cpp
namespace lv {

// Render obj and its children into a new DrawBuffer (useful for
// animation thumbnails, canvas widgets, etc.)
template<typename Pixel = color::ARGB8888::pixel_type>
[[nodiscard]] DrawBuffer<Pixel>
snapshot(const Object& obj,
         ColorFormat fmt = ColorFormat::ARGB8888);

} // namespace lv
```
