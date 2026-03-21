# 10 – Porting / Migration Guide

## 1. Overview

lvgl-cxx is a **breaking-change redesign** — there is no automatic migration.
This document maps every common LVGL v9.5.0 C pattern to its lvgl-cxx C++23
equivalent.

## 2. Initialization

| C                                              | C++23                                                |
|------------------------------------------------|------------------------------------------------------|
| `lv_init()`                                    | Constructor of `lv::Display`                         |
| `lv_deinit()`                                  | Destructor of `lv::Display`                          |
| `lv_tick_inc(ms)`                              | `lv::tick_increment(ms)` or `lv::Ticker` RAII object |
| `lv_timer_handler()`                           | `display.refresh()` (or automatic via tick thread)   |
| `lv_display_create(w, h)`                      | `lv::Display display{w, h, flush_cb}`                |
| `lv_display_set_flush_cb(disp, cb)`            | passed as constructor argument                       |
| `lv_display_set_draw_buffers(d, b1, b2, n, …)` | `display.set_draw_buffers(span1, span2)`             |

### Before (C):
```c
lv_init();
lv_display_t *disp = lv_display_create(480, 320);
lv_display_set_flush_cb(disp, my_flush_cb);

static uint16_t buf1[480 * 32];
lv_display_set_draw_buffers(disp, buf1, NULL, sizeof(buf1),
                            LV_DISPLAY_RENDER_MODE_PARTIAL);
```

### After (C++23):
```cpp
static std::array<lv::color::RGB565::pixel_type, 480 * 32> buf1;

lv::Display display{480, 320,
    [](lv::Display& d, const lv::Area& area,
       std::mdspan<const lv::color::RGB565::pixel_type,
                   std::dextents<int,2>> px) {
        my_flush_to_hardware(area, px);
    }
};
display.set_draw_buffers(std::span{buf1});
```

---

## 3. Object / Widget Creation

| C                          | C++23                                           |
|----------------------------|-------------------------------------------------|
| `lv_obj_create(parent)`    | `parent.create<lv::Object>()`                   |
| `lv_label_create(parent)`  | `parent.create<lv::Label>()`                    |
| `lv_button_create(parent)` | `parent.create<lv::Button>()`                   |
| `lv_obj_delete(obj)`       | `parent.remove_child(obj)` or parent destructor |

### Before:
```c
lv_obj_t *btn = lv_button_create(lv_screen_active());
lv_obj_set_size(btn, 120, 50);
lv_obj_center(btn);

lv_obj_t *lbl = lv_label_create(btn);
lv_label_set_text(lbl, "Click me");
lv_obj_center(lbl);
```

### After:
```cpp
auto& btn = display.active_screen().create<lv::Button>()
    .set_size(120, 50)
    .align(lv::Align::Center);

btn.create<lv::Label>()
   .set_text("Click me")
   .align(lv::Align::Center);
```

---

## 4. Flags and States

| C                                                | C++23                                      |
|--------------------------------------------------|--------------------------------------------|
| `lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN)`       | `obj.add_flag(lv::ObjFlags::Hidden)`       |
| `lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE)` | `obj.remove_flag(lv::ObjFlags::Clickable)` |
| `lv_obj_has_flag(obj, f)`                        | `obj.has_flag(f)`                          |
| `lv_obj_add_state(obj, LV_STATE_CHECKED)`        | `obj.add_state(lv::ObjState::Checked)`     |
| `lv_obj_has_state(obj, LV_STATE_DISABLED)`       | `obj.has_state(lv::ObjState::Disabled)`    |

---

## 5. Styles

| C                                                       | C++23                                                           |
|---------------------------------------------------------|-----------------------------------------------------------------|
| `lv_style_t style; lv_style_init(&style);`              | `lv::Style style;`                                              |
| `lv_style_set_bg_color(&style, lv_color_hex(0xFF0000))` | `style.set(lv::prop::BgColor{}, lv::Color::from_hex(0xFF0000))` |
| `lv_style_set_radius(&style, 8)`                        | `style.set(lv::prop::Radius{}, 8)`                              |
| `lv_obj_add_style(obj, &style, 0)`                      | `obj.add_style(style)`                                          |
| `lv_obj_add_style(obj, &style, LV_STATE_PRESSED)`       | `obj.add_style(style, lv::StyleSelector::Pressed)`              |
| `lv_obj_add_style(obj, &style, LV_PART_SCROLLBAR)`      | `obj.add_style(style, {lv::Part::Scrollbar})`                   |
| `lv_obj_remove_style_all(obj)`                          | `obj.remove_all_styles()`                                       |
| `lv_obj_set_style_bg_color(obj, c, 0)`                  | `obj.set_style_bg_color(c)`                                     |

### Before:
```c
static lv_style_t style_btn;
lv_style_init(&style_btn);
lv_style_set_bg_color(&style_btn, lv_color_hex(0x2196F3));
lv_style_set_bg_opa(&style_btn, LV_OPA_COVER);
lv_style_set_radius(&style_btn, 8);
lv_style_set_pad_all(&style_btn, 10);
lv_obj_add_style(btn, &style_btn, 0);
```

### After:
```cpp
lv::Style btn_style;
btn_style
    .set(lv::prop::BgColor{},   lv::Color::from_hex(0x2196F3))
    .set(lv::prop::BgOpacity{}, lv::OpaFull)
    .set(lv::prop::Radius{},    8)
    .set(lv::prop::PadLeft{},   10)
    .set(lv::prop::PadRight{},  10)
    .set(lv::prop::PadTop{},    10)
    .set(lv::prop::PadBottom{}, 10);
btn.add_style(btn_style);
```

---

## 6. Events

| C                                                      | C++23                                           |
|--------------------------------------------------------|-------------------------------------------------|
| `lv_obj_add_event_cb(obj, cb, LV_EVENT_CLICKED, data)` | `obj.on(lv::EventCode::Clicked, handler)`       |
| `lv_obj_remove_event_cb(obj, cb)`                      | destroy `EventHandle` or call `handle.remove()` |
| `lv_event_get_target(e)`                               | `e.target()`                                    |
| `lv_event_get_user_data(e)`                            | not needed – use lambda capture                 |
| `lv_event_get_code(e)`                                 | `e.code()`                                      |
| `lv_event_get_key(e)`                                  | `e.key()`                                       |
| `lv_event_get_indev(e)`                                | `e.indev()`                                     |
| `lv_event_stop_processing(e)`                          | `e.stop()`                                      |

### Before:
```c
static void btn_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    my_data_t *data = lv_event_get_user_data(e);
    if (code == LV_EVENT_CLICKED) {
        data->count++;
        update_label(data);
    }
}
lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, &my_data);
```

### After:
```cpp
btn.on(lv::EventCode::Clicked, [this](lv::Event& e) {
    count_++;
    update_label();
}).release();
```

---

## 7. Animations

| C                                                | C++23                               |
|--------------------------------------------------|-------------------------------------|
| `lv_anim_t a; lv_anim_init(&a);`                 | `lv::Animation a;`                  |
| `lv_anim_set_exec_cb(&a, cb)`                    | `a.set_exec(fn)`                    |
| `lv_anim_set_var(&a, obj)`                       | captured in lambda                  |
| `lv_anim_set_values(&a, 0, 100)`                 | `a.set_range(0, 100)`               |
| `lv_anim_set_duration(&a, 500)`                  | `a.set_duration(500ms)`             |
| `lv_anim_set_path_cb(&a, lv_anim_path_ease_out)` | `a.set_easing(lv::Easing::EaseOut)` |
| `lv_anim_start(&a)`                              | `a.start()`                         |

### Before:
```c
lv_anim_t a;
lv_anim_init(&a);
lv_anim_set_var(&a, obj);
lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_x);
lv_anim_set_values(&a, 0, 200);
lv_anim_set_duration(&a, 500);
lv_anim_start(&a);
```

### After:
```cpp
using namespace std::chrono_literals;
lv::Animation{}.
    set_range(0, 200).
    set_duration(500ms).
    set_easing(lv::Easing::Linear).
    set_exec([&obj](int32_t v) { obj.set_x(v); }).
    start();
```

---

## 8. Input Devices

| C                                                 | C++23                                       |
|---------------------------------------------------|---------------------------------------------|
| `lv_indev_create()`                               | `lv::Pointer indev;` / `lv::Encoder indev;` |
| `lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER)` | type encoded in class                       |
| `lv_indev_set_read_cb(indev, cb)`                 | `indev.set_read([](lv::InputState& s){…})`  |

---

## 9. Groups (Focus Management)

| C                              | C++23                       |
|--------------------------------|-----------------------------|
| `lv_group_create()`            | `lv::Group group;`          |
| `lv_group_add_obj(g, obj)`     | `group.add(obj)`            |
| `lv_group_remove_obj(obj)`     | `group.remove(obj)`         |
| `lv_group_set_default(g)`      | `group.set_as_default()`    |
| `lv_group_focus_next(g)`       | `group.focus_next()`        |
| `lv_group_focus_prev(g)`       | `group.focus_prev()`        |
| `lv_group_get_focused(g)`      | `group.focused()`           |
| `lv_group_set_focus_cb(g, cb)` | `group.on_focus_change(fn)` |

---

## 10. Logging

| C                              | C++23                                        |
|--------------------------------|----------------------------------------------|
| `LV_LOG_INFO("msg %d", val)`   | `lv::log(lv::LogLevel::Info, "msg {}", val)` |
| `LV_LOG_WARN(…)`               | `lv::log(lv::LogLevel::Warn, …)`             |
| `LV_LOG_ERROR(…)`              | `lv::log(lv::LogLevel::Error, …)`            |
| `lv_log_register_print_cb(cb)` | `lv::set_log_sink(fn)`                       |

---

## 11. File System

| C                                       | C++23                                                    |
|-----------------------------------------|----------------------------------------------------------|
| `lv_fs_drv_t drv; lv_fs_drv_init(&drv)` | implement `lv::FsDrv` concept                            |
| `lv_fs_drv_register(&drv)`              | `lv::fs_register(drv)`                                   |
| `lv_fs_open(…)`                         | `lv::fs_open(path, mode)` → `std::expected<File, Error>` |

---

## 12. Fonts

| C                                   | C++23                                    |
|-------------------------------------|------------------------------------------|
| `lv_font_t`                         | `lv::Font` (non-owning view)             |
| `lv_freetype_font_create(…)`        | `lv::FreeTypeFont::create(path, size)`   |
| `lv_style_set_text_font(&s, &font)` | `style.set(lv::prop::TextFont{}, &font)` |
