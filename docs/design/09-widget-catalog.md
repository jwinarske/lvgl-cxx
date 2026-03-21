# 09 – Widget Catalog

All 30+ LVGL v9.5.0 widgets are redesigned as first-class C++23 classes.  Each
inherits (directly or transitively) from `lv::Object`.

## Naming Convention

| C (LVGL v9.5.0)                         | C++23 (lvgl-cxx)   |
|-----------------------------------------|--------------------|
| `lv_label_t` / `lv_label_*`             | `lv::Label`        |
| `lv_button_t` / `lv_button_*`           | `lv::Button`       |
| `lv_slider_t` / `lv_slider_*`           | `lv::Slider`       |
| `lv_arc_t` / `lv_arc_*`                 | `lv::Arc`          |
| `lv_bar_t` / `lv_bar_*`                 | `lv::Bar`          |
| `lv_switch_t` / `lv_switch_*`           | `lv::Switch`       |
| `lv_checkbox_t` / `lv_checkbox_*`       | `lv::Checkbox`     |
| `lv_dropdown_t` / `lv_dropdown_*`       | `lv::Dropdown`     |
| `lv_roller_t` / `lv_roller_*`           | `lv::Roller`       |
| `lv_textarea_t` / `lv_textarea_*`       | `lv::TextArea`     |
| `lv_keyboard_t` / `lv_keyboard_*`       | `lv::Keyboard`     |
| `lv_image_t` / `lv_image_*`             | `lv::Image`        |
| `lv_animimage_t` / `lv_animimage_*`     | `lv::AnimImage`    |
| `lv_imagebutton_t` / `lv_imagebutton_*` | `lv::ImageButton`  |
| `lv_label_t` arc variant                | `lv::ArcLabel`     |
| `lv_line_t` / `lv_line_*`               | `lv::Line`         |
| `lv_canvas_t` / `lv_canvas_*`           | `lv::Canvas`       |
| `lv_chart_t` / `lv_chart_*`             | `lv::Chart`        |
| `lv_table_t` / `lv_table_*`             | `lv::Table`        |
| `lv_buttonmatrix_t`                     | `lv::ButtonMatrix` |
| `lv_scale_t` / `lv_scale_*`             | `lv::Scale`        |
| `lv_led_t` / `lv_led_*`                 | `lv::Led`          |
| `lv_list_t` / `lv_list_*`               | `lv::List`         |
| `lv_msgbox_t` / `lv_msgbox_*`           | `lv::MsgBox`       |
| `lv_calendar_t` / `lv_calendar_*`       | `lv::Calendar`     |
| `lv_span_t` / `lv_span_*`               | `lv::Span`         |
| `lv_spinbox_t` / `lv_spinbox_*`         | `lv::SpinBox`      |
| `lv_spinner_t` / `lv_spinner_*`         | `lv::Spinner`      |
| `lv_tabview_t` / `lv_tabview_*`         | `lv::TabView`      |
| `lv_tileview_t` / `lv_tileview_*`       | `lv::TileView`     |
| `lv_win_t` / `lv_win_*`                 | `lv::Window`       |
| `lv_menu_t` / `lv_menu_*`               | `lv::Menu`         |

---

## `lv::Label`

```cpp
class Label : public Object {
public:
    explicit Label(Object* parent);

    Label& set_text(std::string_view text);     // static – zero alloc
    Label& set_text(std::string text);          // dynamic copy
    Label& set_text_fmt(std::format_string<auto...> fmt, auto&&... args);
    Label& set_long_mode(LabelLongMode mode);
    Label& set_text_align(TextAlign align);
    Label& set_recolor(bool en);                // inline color codes
    Label& set_text_selection(bool en);

    [[nodiscard]] std::string_view text() const noexcept;
    [[nodiscard]] LabelLongMode    long_mode() const noexcept;
};
```

---

## `lv::Button`

```cpp
class Button : public Object {
public:
    explicit Button(Object* parent);
    // Inherits all Object geometry / style / event API.
    // No extra setters needed for a plain button; content comes from children.
};
```

---

## `lv::Slider`

```cpp
class Slider : public Object {
public:
    explicit Slider(Object* parent);

    Slider& set_value(int32_t value, AnimEnable anim = AnimEnable::Off);
    Slider& set_left_value(int32_t value, AnimEnable anim = AnimEnable::Off);
    Slider& set_range(int32_t min, int32_t max);
    Slider& set_mode(SliderMode mode);

    [[nodiscard]] int32_t    value()      const noexcept;
    [[nodiscard]] int32_t    left_value() const noexcept;
    [[nodiscard]] int32_t    min_value()  const noexcept;
    [[nodiscard]] int32_t    max_value()  const noexcept;
    [[nodiscard]] SliderMode mode()       const noexcept;
    [[nodiscard]] bool       is_dragged() const noexcept;
};
```

---

## `lv::Arc`

```cpp
class Arc : public Object {
public:
    explicit Arc(Object* parent);

    Arc& set_start_angle(int32_t angle);
    Arc& set_end_angle(int32_t angle);
    Arc& set_bg_start_angle(int32_t angle);
    Arc& set_bg_end_angle(int32_t angle);
    Arc& set_value(int32_t value);
    Arc& set_range(int32_t min, int32_t max);
    Arc& set_mode(ArcMode mode);
    Arc& set_rotation(int32_t rotation);

    [[nodiscard]] int32_t angle_start() const noexcept;
    [[nodiscard]] int32_t angle_end()   const noexcept;
    [[nodiscard]] int32_t value()       const noexcept;
    [[nodiscard]] int32_t min_value()   const noexcept;
    [[nodiscard]] int32_t max_value()   const noexcept;
};
```

---

## `lv::Bar`

```cpp
class Bar : public Object {
public:
    explicit Bar(Object* parent);

    Bar& set_value(int32_t value, AnimEnable anim = AnimEnable::Off);
    Bar& set_start_value(int32_t value, AnimEnable anim = AnimEnable::Off);
    Bar& set_range(int32_t min, int32_t max);
    Bar& set_mode(BarMode mode);

    [[nodiscard]] int32_t value()       const noexcept;
    [[nodiscard]] int32_t start_value() const noexcept;
    [[nodiscard]] int32_t min_value()   const noexcept;
    [[nodiscard]] int32_t max_value()   const noexcept;
};
```

---

## `lv::Switch`

```cpp
class Switch : public Object {
public:
    explicit Switch(Object* parent);

    Switch& set_checked(bool checked, AnimEnable anim = AnimEnable::On);
    [[nodiscard]] bool is_checked() const noexcept;
};
```

---

## `lv::Checkbox`

```cpp
class Checkbox : public Object {
public:
    explicit Checkbox(Object* parent);

    Checkbox& set_text(std::string_view text);
    Checkbox& set_text(std::string text);
    [[nodiscard]] std::string_view text() const noexcept;
};
```

---

## `lv::Dropdown`

```cpp
class Dropdown : public Object {
public:
    explicit Dropdown(Object* parent);

    Dropdown& set_options(std::string_view opts);   // "\n"-separated list
    Dropdown& set_options(std::span<const std::string_view> opts);
    Dropdown& set_selected(uint32_t index);
    Dropdown& set_dir(Dir dir);
    Dropdown& set_text(std::string_view text);
    Dropdown& set_symbol(std::string_view symbol);

    [[nodiscard]] uint32_t selected()           const noexcept;
    [[nodiscard]] std::string selected_str()    const;
    [[nodiscard]] uint32_t   option_count()     const noexcept;

    void open();
    void close();
    [[nodiscard]] bool is_open() const noexcept;
};
```

---

## `lv::TextArea`

```cpp
class TextArea : public Object {
public:
    explicit TextArea(Object* parent);

    TextArea& set_text(std::string_view text);
    TextArea& add_char(uint32_t c);
    TextArea& add_text(std::string_view s);
    TextArea& delete_char();
    TextArea& set_placeholder_text(std::string_view text);
    TextArea& set_accepted_chars(std::string_view chars);
    TextArea& set_max_length(uint32_t len);
    TextArea& set_password_mode(bool en);
    TextArea& set_one_line(bool en);
    TextArea& set_cursor_pos(int32_t pos);

    [[nodiscard]] std::string_view text()        const noexcept;
    [[nodiscard]] int32_t          cursor_pos()  const noexcept;
    [[nodiscard]] bool             is_password() const noexcept;
};
```

---

## `lv::Image`

```cpp
class Image : public Object {
public:
    explicit Image(Object* parent);

    Image& set_src(const void* src);          // lv_image_dsc_t or file path
    Image& set_src(std::string_view path);    // filesystem path
    Image& set_offset(int32_t x, int32_t y);
    Image& set_scale(uint32_t zoom);          // 256 = 1:1
    Image& set_scale_x(uint32_t zoom);
    Image& set_scale_y(uint32_t zoom);
    Image& set_rotation(int32_t angle);       // tenths of degrees
    Image& set_pivot(int32_t x, int32_t y);
    Image& set_blend_mode(BlendMode mode);
    Image& set_antialias(bool en);
    Image& set_inner_align(ImageAlign align);

    [[nodiscard]] const void* src()      const noexcept;
    [[nodiscard]] int32_t     offset_x() const noexcept;
    [[nodiscard]] int32_t     offset_y() const noexcept;
};
```

---

## `lv::Chart`

```cpp
class Chart : public Object {
public:
    explicit Chart(Object* parent);

    Chart& set_type(ChartType type);
    Chart& set_point_count(uint32_t count);
    Chart& set_range(ChartAxis axis, int32_t min, int32_t max);
    Chart& set_update_mode(ChartUpdateMode mode);
    Chart& set_div_line_count(uint8_t h_div, uint8_t v_div);

    // Series management
    ChartSeries& add_series(Color color, ChartAxis axis);
    void         remove_series(ChartSeries& series);
    void         hide_series(ChartSeries& series, bool hide);

    void set_next_value(ChartSeries& series, int32_t value);
    void set_value_by_id(ChartSeries& series, uint32_t id, int32_t value);

    void refresh();

    [[nodiscard]] uint32_t        point_count()  const noexcept;
    [[nodiscard]] uint32_t        pressed_point() const noexcept;
    [[nodiscard]] ChartType       type()          const noexcept;
};
```

---

## `lv::Table`

```cpp
class Table : public Object {
public:
    explicit Table(Object* parent);

    Table& set_cell_value(uint32_t row, uint32_t col, std::string_view text);
    Table& set_cell_value_fmt(uint32_t row, uint32_t col,
                              std::format_string<auto...> fmt, auto&&... args);
    Table& set_row_count(uint32_t rows);
    Table& set_column_count(uint32_t cols);
    Table& set_column_width(uint32_t col, int32_t w);
    Table& set_cell_ctrl(uint32_t row, uint32_t col, TableCellCtrl ctrl);

    [[nodiscard]] std::string_view cell_value(uint32_t row, uint32_t col) const;
    [[nodiscard]] uint32_t         row_count()    const noexcept;
    [[nodiscard]] uint32_t         column_count() const noexcept;

    // Ranges
    [[nodiscard]] auto rows()    noexcept; // range over row indices
    [[nodiscard]] auto columns() noexcept;
};
```

---

## `lv::Keyboard`

```cpp
class Keyboard : public Object {
public:
    explicit Keyboard(Object* parent);

    Keyboard& set_textarea(TextArea& ta);
    Keyboard& set_mode(KeyboardMode mode);
    Keyboard& set_map(KeyboardMode mode,
                      std::span<const std::string_view> map,
                      std::span<const ButtonCtrl> ctrl_map);

    [[nodiscard]] TextArea*     textarea()   noexcept;
    [[nodiscard]] KeyboardMode  mode()  const noexcept;
};
```
