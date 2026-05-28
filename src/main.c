#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>

#include "bar.h"
#include "widget.h"
#include "config.h"

static volatile int running = 1;
static volatile int reload = 0;

static void sig_handler(int sig)
{
    if (sig == SIGHUP)
        reload = 1;
    else
        running = 0;
}

static void bars_destroy(Bar **bars)
{
    Bar *bar = *bars;
    while (bar != NULL) {
        Bar *next = bar->next;
        bar_destroy(bar);
        bar = next;
    }
    *bars = NULL;
}

static int bars_create(Display *dpy, int screen, Config *cfg, Bar **bars)
{
    Bar *last = NULL;
    *bars = NULL;

    for (ConfigBlock *bc = cfg->bars; bc != NULL; bc = bc->next) {
        Bar *bar = bar_create(dpy, screen, bc);
        if (bar != NULL) {
            if (*bars == NULL)
                *bars = bar;
            else
                last->next = bar;
            last = bar;
        }
    }

    return *bars != NULL;
}

int main(int argc, char *argv[])
{
    const char *config_path = getenv("OXBAR_CONFIG");
    if (config_path == NULL) {
        config_path = getenv("HOME");
        char path[256];
        snprintf(path, sizeof(path), "%s/.config/oxbar/config", config_path);
        config_path = strdup(path);
    }

    if (argc > 1)
        config_path = argv[1];

    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);
    signal(SIGHUP, sig_handler);

    Config *cfg = config_parse(config_path);
    if (cfg == NULL) {
        fprintf(stderr, "oxbar: failed to parse config\n");
        return 1;
    }

    Display *dpy = XOpenDisplay(NULL);
    if (dpy == NULL) {
        fprintf(stderr, "oxbar: cannot open display\n");
        config_free(cfg);
        return 1;
    }

    int screen = DefaultScreen(dpy);
    Bar *bars = NULL;

    if (!bars_create(dpy, screen, cfg, &bars)) {
        fprintf(stderr, "oxbar: no bars configured\n");
        XCloseDisplay(dpy);
        config_free(cfg);
        return 1;
    }

    struct timespec ts = { .tv_sec = 0, .tv_nsec = 100000000 };
    struct timespec tick;
    clock_gettime(CLOCK_MONOTONIC, &tick);

    while (running) {
        if (reload) {
            reload = 0;
            bars_destroy(&bars);
            config_free(cfg);
            cfg = config_parse(config_path);
            if (cfg == NULL) {
                fprintf(stderr, "oxbar: config reload failed\n");
                break;
            }
            bars_create(dpy, screen, cfg, &bars);
            fprintf(stderr, "oxbar: config reloaded\n");
        }

        double now = tick.tv_sec + tick.tv_nsec / 1e9;
        for (Bar *bar = bars; bar != NULL; bar = bar->next) {
            for (int i = 0; i < bar->widget_count; i++) {
                Widget *w = bar->widgets[i];
                if (w->interval > 0 && now - w->last_update >= w->interval) {
                    widget_update(w);
                    w->last_update = now;
                }
            }
        }

        for (Bar *bar = bars; bar != NULL; bar = bar->next)
            bar_render(bar);

        while (XPending(dpy) > 0) {
            XEvent ev;
            XNextEvent(dpy, &ev);
            if (ev.type == ButtonPress) {
                for (Bar *bar = bars; bar != NULL; bar = bar->next) {
                    if (ev.xbutton.window != bar->win)
                        continue;
                    if (ev.xbutton.button == 4 || ev.xbutton.button == 5)
                        bar_scroll(bar, ev.xbutton.x, ev.xbutton.button);
                    else
                        bar_click(bar, ev.xbutton.x);
                }
            }
        }

        XFlush(dpy);
        nanosleep(&ts, NULL);
        clock_gettime(CLOCK_MONOTONIC, &tick);
    }

    bars_destroy(&bars);
    XCloseDisplay(dpy);
    config_free(cfg);

    return 0;
}
