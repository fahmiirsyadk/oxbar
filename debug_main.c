#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include "bar.h"
#include "widget.h"
#include "config.h"

int main(int argc, char *argv[]) {
    fprintf(stderr, "DEBUG: starting\n");
    Config *cfg = config_parse("/home/xo/.config/oxbar/config");
    if (cfg == NULL) { fprintf(stderr, "DEBUG: config_parse returned NULL\n"); return 1; }
    fprintf(stderr, "DEBUG: config parsed OK\n");

    Display *dpy = XOpenDisplay(NULL);
    if (dpy == NULL) { fprintf(stderr, "DEBUG: no display\n"); config_free(cfg); return 1; }
    fprintf(stderr, "DEBUG: display opened\n");

    int screen = DefaultScreen(dpy);
    Bar *bars = NULL, *last = NULL;
    for (ConfigBlock *bc = cfg->bars; bc; bc = bc->next) {
        fprintf(stderr, "DEBUG: creating bar '%s'\n", bc->name);
        Bar *b = bar_create(dpy, screen, bc);
        if (b) {
            fprintf(stderr, "DEBUG: bar created ok, widgets=%d, width=%d\n", b->widget_count, b->width);
            if (!bars) bars = b; else last->next = b;
            last = b;
        }
    }
    if (!bars) { fprintf(stderr, "DEBUG: no bars\n"); return 1; }

    fprintf(stderr, "DEBUG: rendering once...\n");
    for (Bar *b = bars; b; b = b->next) bar_render(b);
    XFlush(dpy);
    fprintf(stderr, "DEBUG: render done, sleeping 2s...\n");
    sleep(2);
    for (Bar *b = bars; b; b = b->next) bar_destroy(b);
    XCloseDisplay(dpy);
    config_free(cfg);
    fprintf(stderr, "DEBUG: done\n");
    return 0;
}
