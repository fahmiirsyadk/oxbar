#ifndef OXBAR_H
#define OXBAR_H

#include <stddef.h>
#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>

typedef void (*WidgetUpdate)(void *ctx, char *buf, size_t len);

typedef enum { RENDER_TEXT, RENDER_PROGRESS } RenderMode;

typedef struct Widget {
    char *name;
    char *icon;
    char *label;
    char *click_cmd;
    char *scroll_up;
    char *scroll_down;
    char *fg;
    char *bg;
    RenderMode render;
    double interval;
    WidgetUpdate update;
    void *ctx;
    double last_update;
    int x;
    int w;
} Widget;

Widget *widget_create(const char *name, const char *icon,
                      double interval, WidgetUpdate update, void *ctx);
void widget_destroy(Widget *widget);
void widget_update(Widget *widget);

void run_cmd(const char *cmd, char *buf, size_t len);
void update_cmd(void *ctx, char *buf, size_t len);

Widget *widget_cmd_create(const char *name, const char *icon,
                          const char *cmd, double interval);

#endif
