#ifndef BAR_H
#define BAR_H

#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>

typedef struct Widget Widget;
typedef struct Config Config;
typedef struct ConfigBlock ConfigBlock;

typedef enum {
    BAR_ALIGN_LEFT,
    BAR_ALIGN_CENTER,
    BAR_ALIGN_RIGHT,
    BAR_ALIGN_STRETCH
} BarAlign;

typedef struct Bar {
    Display *dpy;
    int screen;
    Window win;
    Pixmap buf;
    GC gc;
    XftDraw *draw;
    XftFont *font;
    int x;
    int y;
    int width;
    int height;
    BarAlign alignment;
    char *fg;
    char *bg;
    char *sep_color;
    int padding;
    Widget **widgets;
    int widget_count;
    struct Bar *next;
} Bar;

Bar *bar_create(Display *dpy, int screen, ConfigBlock *cfg);
void bar_destroy(Bar *bar);
void bar_render(Bar *bar);
void bar_click(Bar *bar, int x);

#endif
