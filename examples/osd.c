#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include "ox.h"

typedef struct {
    OxWindow *win;
    OxWidget *widget;
    int shown;
    time_t hide_at;
    int timeout;
} OSD;

static OSD vol_osd;
static OSD bright_osd;
static volatile int show_vol = 0;
static volatile int show_bright = 0;

static void sig_handler(int sig) {
    if (sig == SIGUSR1) show_vol = 1;
    else if (sig == SIGUSR2) show_bright = 1;
}

static void osd_show(OSD *osd) {
    time_t now = time(NULL);
    if (osd->shown && (now - (osd->hide_at - osd->timeout)) < 1) return;
    if (!osd->shown) ox_window_show(osd->win);
    osd->shown = 1;
    osd->hide_at = now + osd->timeout;
    ox_widget_update(osd->widget);
}

static void osd_hide(OSD *osd) {
    if (osd->shown) {
        ox_window_hide(osd->win);
        osd->shown = 0;
    }
}

static void osd_render(OSD *osd) {
    if (!osd->shown) return;
    int w = 30, h = 200;
    ox_draw_rect(osd->win, 0, 0, w, h, "#111111");
    const char *label = ox_widget_get_label(osd->widget);
    int pct = atoi(label);
    if (pct < 0) pct = 0;
    if (pct > 100) pct = 100;
    int fill = (h - 4) * pct / 100;
    ox_draw_rect(osd->win, 2, h - 2 - fill, w - 4, fill, "#4488ff");
}

static void vol_update(void *ctx, char *buf, size_t len) {
    (void)ctx;
    FILE *f = popen("pactl get-sink-volume @DEFAULT_SINK@ 2>/dev/null | grep -oP '\\d+%' | head -1 | grep -oP '\\d+'", "r");
    if (!f) { snprintf(buf, len, "0"); return; }
    if (fgets(buf, len, f)) {
        size_t n = strlen(buf);
        while (n > 0 && (buf[n-1] == '\n' || buf[n-1] == '\r')) buf[--n] = '\0';
    } else {
        snprintf(buf, len, "0");
    }
    pclose(f);
}

static void bright_update(void *ctx, char *buf, size_t len) {
    (void)ctx;
    FILE *f = fopen("/sys/class/backlight/backlight/brightness", "r");
    if (!f) { snprintf(buf, len, "0"); return; }
    long cur = 0, max = 15360;
    fscanf(f, "%ld", &cur);
    fclose(f);
    snprintf(buf, len, "%ld", cur * 100 / max);
}

int main(void) {
    struct sigaction sa = { .sa_handler = sig_handler, .sa_flags = SA_RESTART };
    sigemptyset(&sa.sa_mask);
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);

    ox_init();
    int sh = DisplayHeight(ox_display(), ox_screen());

    vol_osd.win = ox_window_new(20, (sh - 200) / 2, 30, 200);
    ox_window_set_bg(vol_osd.win, "#111111");
    vol_osd.widget = ox_widget_new("vol", 0.1);
    ox_widget_set_update(vol_osd.widget, vol_update, NULL);
    vol_osd.timeout = 3;
    ox_window_hide(vol_osd.win);

    bright_osd.win = ox_window_new(60, (sh - 200) / 2, 30, 200);
    ox_window_set_bg(bright_osd.win, "#111111");
    bright_osd.widget = ox_widget_new("bright", 0.1);
    ox_widget_set_update(bright_osd.widget, bright_update, NULL);
    bright_osd.timeout = 3;
    ox_window_hide(bright_osd.win);

    struct timespec ts = { .tv_sec = 0, .tv_nsec = 100000000 };
    struct timespec now_ts;
    clock_gettime(CLOCK_MONOTONIC, &now_ts);

    for (;;) {
        if (show_vol) { show_vol = 0; osd_show(&vol_osd); }
        if (show_bright) { show_bright = 0; osd_show(&bright_osd); }

        time_t now = time(NULL);
        if (vol_osd.shown && now >= vol_osd.hide_at) osd_hide(&vol_osd);
        if (bright_osd.shown && now >= bright_osd.hide_at) osd_hide(&bright_osd);

        clock_gettime(CLOCK_MONOTONIC, &now_ts);
        double cur = now_ts.tv_sec + now_ts.tv_nsec / 1e9;

        if (vol_osd.shown && cur - ox_widget_get_last_update(vol_osd.widget) >= 0.1) {
            ox_widget_update(vol_osd.widget);
            ox_widget_set_last_update(vol_osd.widget, cur);
        }
        if (bright_osd.shown && cur - ox_widget_get_last_update(bright_osd.widget) >= 0.1) {
            ox_widget_update(bright_osd.widget);
            ox_widget_set_last_update(bright_osd.widget, cur);
        }

        osd_render(&vol_osd);
        osd_render(&bright_osd);

        Display *dpy = ox_display();
        while (XPending(dpy) > 0) { XEvent ev; XNextEvent(dpy, &ev); }
        XFlush(dpy);
        nanosleep(&ts, NULL);
    }

    return 0;
}
