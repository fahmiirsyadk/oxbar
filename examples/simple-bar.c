#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "ox.h"

typedef struct {
    OxWindow *win;
    OxWidget *widgets[16];
    int count;
    int height;
    int padding;
    const char *fg;
    const char *bg;
    const char *sep;
} Bar;

static void bar_add(Bar *bar, OxWidget *w) {
    bar->widgets[bar->count++] = w;
}

static void bar_render(Bar *bar) {
    int sw = DisplayWidth(ox_display(), ox_screen());
    int cx = bar->padding;
    int cy = bar->height / 2 + ox_text_width(bar->win, "X") / 3;

    ox_draw_rect(bar->win, 0, 0, sw, bar->height, bar->bg);

    for (int i = 0; i < bar->count; i++) {
        if (i > 0) {
            ox_draw_text(bar->win, cx, cy, " | ", bar->sep);
            cx += ox_text_width(bar->win, " | ") + bar->padding;
        }
        const char *icon = ox_widget_get_icon(bar->widgets[i]);
        const char *label = ox_widget_get_label(bar->widgets[i]);
        if (icon) {
            ox_draw_text(bar->win, cx, cy, icon, bar->fg);
            cx += ox_text_width(bar->win, icon);
        }
        ox_draw_text(bar->win, cx, cy, label, bar->fg);
        cx += ox_text_width(bar->win, label) + bar->padding;
    }
}

static void time_update(void *ctx, char *buf, size_t len) {
    (void)ctx;
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    snprintf(buf, len, "%02d:%02d:%02d", tm->tm_hour, tm->tm_min, tm->tm_sec);
}

static void cpu_update(void *ctx, char *buf, size_t len) {
    (void)ctx;
    FILE *f = fopen("/proc/stat", "r");
    if (!f) { snprintf(buf, len, "??%%"); return; }
    char line[256];
    fgets(line, sizeof(line), f);
    fclose(f);
    unsigned long long u, n, s, i, w, q, sq;
    sscanf(line, "cpu %llu %llu %llu %llu %llu %llu %llu", &u, &n, &s, &i, &w, &q, &sq);
    unsigned long long total = u + n + s + i + w + q + sq;
    unsigned long long idle = i;
    static unsigned long long prev_total = 0, prev_idle = 0;
    unsigned long long dt_total = total - prev_total;
    unsigned long long dt_idle = idle - prev_idle;
    prev_total = total;
    prev_idle = idle;
    int pct = (dt_total > 0) ? (int)((dt_total - dt_idle) * 100 / dt_total) : 0;
    snprintf(buf, len, "%d%%", pct);
}

static void mem_update(void *ctx, char *buf, size_t len) {
    (void)ctx;
    FILE *f = fopen("/proc/meminfo", "r");
    if (!f) { snprintf(buf, len, "??MB"); return; }
    long total = 0, avail = 0;
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        if (sscanf(line, "MemTotal: %ld kB", &total) == 1) continue;
        if (sscanf(line, "MemAvailable: %ld kB", &avail) == 1) break;
    }
    fclose(f);
    long used = (total - avail) / 1024;
    snprintf(buf, len, "%ldMB", used);
}

int main(void) {
    ox_init();

    int sw = DisplayWidth(ox_display(), ox_screen());
    Bar bar = {0};
    bar.height = 24;
    bar.padding = 8;
    bar.fg = "#ffffff";
    bar.bg = "#000000";
    bar.sep = "#555555";
    bar.win = ox_window_new(0, 0, sw, bar.height);
    ox_window_set_bg(bar.win, bar.bg);
    ox_window_set_font(bar.win, "monospace:size=11");
    ox_window_set_strut(bar.win, bar.height, 0, 0, 0);
    ox_window_show(bar.win);

    OxWidget *w_time = ox_widget_new("time", 1.0);
    ox_widget_set_icon(w_time, "TIME");
    ox_widget_set_update(w_time, time_update, NULL);
    bar_add(&bar, w_time);

    OxWidget *w_cpu = ox_widget_new("cpu", 2.0);
    ox_widget_set_icon(w_cpu, "CPU");
    ox_widget_set_update(w_cpu, cpu_update, NULL);
    bar_add(&bar, w_cpu);

    OxWidget *w_mem = ox_widget_new("mem", 2.0);
    ox_widget_set_icon(w_mem, "MEM");
    ox_widget_set_update(w_mem, mem_update, NULL);
    bar_add(&bar, w_mem);

    struct timespec ts = { .tv_sec = 0, .tv_nsec = 100000000 };
    struct timespec now_ts;
    clock_gettime(CLOCK_MONOTONIC, &now_ts);

    for (;;) {
        clock_gettime(CLOCK_MONOTONIC, &now_ts);
        double cur = now_ts.tv_sec + now_ts.tv_nsec / 1e9;

        for (int i = 0; i < bar.count; i++) {
            OxWidget *w = bar.widgets[i];
            if (ox_widget_get_interval(w) > 0 &&
                cur - ox_widget_get_last_update(w) >= ox_widget_get_interval(w)) {
                ox_widget_update(w);
                ox_widget_set_last_update(w, cur);
            }
        }

        bar_render(&bar);

        Display *dpy = ox_display();
        while (XPending(dpy) > 0) {
            XEvent ev;
            XNextEvent(dpy, &ev);
        }
        XFlush(dpy);
        nanosleep(&ts, NULL);
    }

    return 0;
}
