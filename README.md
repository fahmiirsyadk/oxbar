# ox

A minimal C toolkit for building X11 status bars and widgets.

No config files. Each example is a standalone C program that defines its own widgets.

## Structure

```
include/ox.h       Core API
src/
  widget.c         Widget lifecycle
  draw.c           Rendering: text, rect, window
  loop.c           Event loop
examples/
  simple-bar.c     Single bar: time + cpu + mem
  multi-bar.c      Left/center/right bars
  vertical-bar.c   Vertical sidebar
  osd.c            OSD with SIGUSR1/SIGUSR2
```

## Build

```bash
make
```

### Dependencies

- libx11, libxft, freetype2, fontconfig

## Examples

```bash
./simple-bar              # single bar
./multi-bar               # three bars
./vertical-bar            # sidebar
./osd &                   # OSD overlay
kill -USR1 $(pidof osd)   # show volume
kill -USR2 $(pidof osd)   # show brightness
```

## API

```c
// Widget
OxWidget *ox_widget_new(const char *name, double interval);
void ox_widget_set_update(OxWidget *w, OxWidgetUpdate update, void *ctx);
void ox_widget_set_icon(OxWidget *w, const char *icon);

// Window
OxWindow *ox_window_new(int x, int y, int w, int h);
void ox_window_set_bg(OxWindow *win, const char *color);
void ox_window_set_font(OxWindow *win, const char *font);
void ox_window_set_strut(OxWindow *win, int top, int bottom, int left, int right);
void ox_window_show(OxWindow *win);
void ox_window_hide(OxWindow *win);

// Draw
void ox_draw_rect(OxWindow *win, int x, int y, int w, int h, const char *color);
void ox_draw_text(OxWindow *win, int x, int y, const char *text, const char *fg);
int ox_text_width(OxWindow *win, const char *text);

// Loop
void ox_init(void);
void ox_run(void);
void ox_quit(void);
```

## License

MIT
