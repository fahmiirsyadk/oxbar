#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <string.h>
#include "ox.h"

struct OxWidget {
    char *name;
    char *icon;
    char label[256];
    char *fg;
    char *bg;
    int render_progress;
    double interval;
    OxWidgetUpdate update;
    OxWidgetClick click;
    void *ctx;
    double last_update;
};

OxWidget *ox_widget_new(const char *name, double interval) {
    OxWidget *w = calloc(1, sizeof(OxWidget));
    w->name = strdup(name);
    w->interval = interval;
    return w;
}

void ox_widget_destroy(OxWidget *w) {
    if (!w) return;
    free(w->name);
    free(w->icon);
    free(w->fg);
    free(w->bg);
    free(w);
}

void ox_widget_set_update(OxWidget *w, OxWidgetUpdate update, void *ctx) {
    w->update = update;
    w->ctx = ctx;
}

void ox_widget_set_click(OxWidget *w, OxWidgetClick click) {
    w->click = click;
}

void ox_widget_set_icon(OxWidget *w, const char *icon) {
    free(w->icon);
    w->icon = icon ? strdup(icon) : NULL;
}

void ox_widget_set_colors(OxWidget *w, const char *fg, const char *bg) {
    free(w->fg);
    free(w->bg);
    w->fg = fg ? strdup(fg) : NULL;
    w->bg = bg ? strdup(bg) : NULL;
}

void ox_widget_set_render_progress(OxWidget *w, int enable) {
    w->render_progress = enable;
}

void ox_widget_update(OxWidget *w) {
    if (w->update)
        w->update(w->ctx, w->label, sizeof(w->label));
}

const char *ox_widget_get_name(OxWidget *w) { return w->name; }
const char *ox_widget_get_label(OxWidget *w) { return w->label; }
const char *ox_widget_get_icon(OxWidget *w) { return w->icon; }
double ox_widget_get_interval(OxWidget *w) { return w->interval; }
double ox_widget_get_last_update(OxWidget *w) { return w->last_update; }
void ox_widget_set_last_update(OxWidget *w, double t) { w->last_update = t; }
