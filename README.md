# ox

A minimal C toolkit for building X11 status bars and widgets.

No config files. Write C, not config.

## Quick Start

```c
#include "ox.h"

int main(void) {
    ox_init();

    OxWindow *win = ox_window_new(0, 0, 200, 28);
    ox_window_set_bg(win, "#000000");
    ox_window_show(win);

    OxWidget *w = ox_widget_new("clock", 1.0);
    ox_widget_set_update(w, my_update, NULL);

    ox_run();
    return 0;
}
```

## Build

```bash
make
```

Dependencies: libx11, libxft, libxpm, freetype2, fontconfig

## API Reference

### ox_init

```c
void ox_init(void);
```

Initialize the ox library. Must be called before any other ox function. Opens the X11 display connection.

### ox_run

```c
void ox_run(void);
```

Start the event loop. Blocks until `ox_quit()` is called or a termination signal is received. Handles X11 events (clicks, exposure) and widget updates.

### ox_quit

```c
void ox_quit(void);
```

Stop the event loop. Can be called from signal handlers or click callbacks.

---

### ox_widget_new

```c
OxWidget *ox_widget_new(const char *name, double interval);
```

Create a new widget.

- `name` — identifier string (used for lookup, logging)
- `interval` — update interval in seconds (0 = never auto-update)

Returns a newly allocated widget. Free with `ox_widget_destroy()`.

### ox_widget_destroy

```c
void ox_widget_destroy(OxWidget *w);
```

Free a widget and its resources.

### ox_widget_set_update

```c
void ox_widget_set_update(OxWidget *w, OxWidgetUpdate update, void *ctx);
```

Set the update callback. Called when the widget's interval elapses.

- `update` — function signature: `void (*)(void *ctx, char *buf, size_t len)`
- `ctx` — opaque pointer passed to the callback

The callback writes its output into `buf` (256 bytes max).

### ox_widget_set_click

```c
void ox_widget_set_click(OxWidget *w, OxWidgetClick click);
```

Set the click callback. Called when the widget is clicked.

- `click` — function signature: `void (*)(void *ctx)`

### ox_widget_set_icon

```c
void ox_widget_set_icon(OxWidget *w, const char *icon);
```

Set an icon prefix. Rendered before the label text. Set to `""` or `NULL` for no icon.

### ox_widget_set_label_text

```c
void ox_widget_set_label_text(OxWidget *w, const char *text);
```

Set a static label. Use for widgets that don't need periodic updates (interval=0).

### ox_widget_set_colors

```c
void ox_widget_set_colors(OxWidget *w, const char *fg, const char *bg);
```

Override per-widget colors. Pass `NULL` to use the bar's default colors.

- `fg` — foreground color (hex, e.g., `"#ff0000"`)
- `bg` — background color (hex, or `NULL` for transparent)

### ox_widget_set_render_progress

```c
void ox_widget_set_render_progress(OxWidget *w, int enable);
```

Enable progress bar rendering. When enabled, the widget's label is interpreted as a percentage (0-100) and rendered as a filled bar.

### ox_widget_update

```c
void ox_widget_update(OxWidget *w);
```

Manually trigger the update callback. Normally called by the event loop based on interval.

### ox_widget_do_click

```c
void ox_widget_do_click(OxWidget *w);
```

Invoke the widget's click callback programmatically.

### ox_widget_get_name

```c
const char *ox_widget_get_name(OxWidget *w);
```

Get the widget's name.

### ox_widget_get_label

```c
const char *ox_widget_get_label(OxWidget *w);
```

Get the widget's current label text (output of the last update callback).

### ox_widget_get_icon

```c
const char *ox_widget_get_icon(OxWidget *w);
```

Get the widget's icon string.

### ox_widget_get_fg

```c
const char *ox_widget_get_fg(OxWidget *w);
```

Get the widget's foreground color override, or `NULL` if using bar default.

### ox_widget_get_bg

```c
const char *ox_widget_get_bg(OxWidget *w);
```

Get the widget's background color override, or `NULL` if using bar default.

### ox_widget_get_interval

```c
double ox_widget_get_interval(OxWidget *w);
```

Get the widget's update interval in seconds.

### ox_widget_get_last_update

```c
double ox_widget_get_last_update(OxWidget *w);
```

Get the timestamp of the last update (monotonic clock, seconds).

### ox_widget_set_last_update

```c
void ox_widget_set_last_update(OxWidget *w, double t);
```

Set the last update timestamp. Used by the event loop to track when to call the update callback.

---

### ox_window_new

```c
OxWindow *ox_window_new(int x, int y, int w, int h);
```

Create a new X11 window. Sets override_redirect (bypasses WM), sets `_NET_WM_WINDOW_TYPE_DOCK`, and prepares an Xft drawing context.

- `x, y` — screen position (pixels)
- `w, h` — window dimensions (pixels)

Returns a newly allocated window. Free with `ox_window_destroy()`.

### ox_window_destroy

```c
void ox_window_destroy(OxWindow *win);
```

Free a window and its resources.

### ox_window_set_bg

```c
void ox_window_set_bg(OxWindow *win, const char *color);
```

Set the background color. Stored for use by `ox_draw_rect()`.

- `color` — hex color string (e.g., `"#000000"`)

### ox_window_set_font

```c
void ox_window_set_font(OxWindow *win, const char *font);
```

Set the Xft font. Affects all subsequent `ox_draw_text()` calls on this window.

- `font` — Xft font name (e.g., `"monospace:size=11"`)

### ox_window_set_click

```c
void ox_window_set_click(OxWindow *win, OxWindowClick click);
```

Set a click handler for the entire window.

- `click` — function signature: `void (*)(OxWindow *win, int x, int y, int button)`

### ox_window_set_strut

```c
void ox_window_set_strut(OxWindow *win, int top, int bottom, int left, int right);
```

Set EWMH struts to reserve screen space. Prevents maximized windows from overlapping your bar.

- `top` — pixels to reserve at top (e.g., bar height)

### ox_window_show

```c
void ox_window_show(OxWindow *win);
```

Map (show) the window.

### ox_window_hide

```c
void ox_window_hide(OxWindow *win);
```

Unmap (hide) the window.

### ox_window_move

```c
void ox_window_move(OxWindow *win, int x, int y);
```

Move the window to a new position.

### ox_window_resize

```c
void ox_window_resize(OxWindow *win, int w, int h);
```

Resize the window.

### ox_window_handle

```c
Window ox_window_handle(OxWindow *win);
```

Get the underlying X11 `Window` handle. Used for matching `XEvent.xbutton.window` in click handlers.

---

### ox_draw_rect

```c
void ox_draw_rect(OxWindow *win, int x, int y, int w, int h, const char *color);
```

Draw a filled rectangle.

- `color` — hex color string (e.g., `"#000000"`)

### ox_draw_text

```c
void ox_draw_text(OxWindow *win, int x, int y, const char *text, const char *fg);
```

Draw text using the window's current font.

- `text` — UTF-8 string
- `fg` — foreground color (hex)

### ox_draw_xpm

```c
void ox_draw_xpm(OxWindow *win, int x, int y, const char *path);
```

Draw an XPM image.

- `path` — filesystem path to `.xpm` file

### ox_draw_sep

```c
void ox_draw_sep(OxWindow *win, int x, int y, const char *sep, const char *fg);
```

Draw a separator string (same as `ox_draw_text`, semantic alias).

### ox_draw_flush

```c
void ox_draw_flush(OxWindow *win);
```

Flush pending X11 drawing commands.

### ox_text_width

```c
int ox_text_width(OxWindow *win, const char *text);
```

Measure the pixel width of a text string using the window's current font.

---

## Signals

| Signal | Action |
|--------|--------|
| `SIGINT` | Quit |
| `SIGTERM` | Quit |
| `SIGHUP` | Quit |
| `SIGUSR1` | Custom (e.g., show volume OSD) |
| `SIGUSR2` | Custom (e.g., show brightness OSD) |

## License

MIT
