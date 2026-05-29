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
    int height, padding;
    const char *fg, *bg, *sep;
} Bar;

static void bar_add(Bar *bar, OxWidget *w) {
    bar->widgets[bar->count++] = w;
}

static int bar_content_width(Bar *bar) {
    int total = bar->padding;
    for (int i = 0; i < bar->count; i++) {
        if (i > 0)
            total += ox_text_width(bar->win, " | ") + bar->padding;
        const char *icon = ox_widget_get_icon(bar->widgets[i]);
        const char *label = ox_widget_get_label(bar->widgets[i]);
        if (icon) total += ox_text_width(bar->win, icon);
        total += ox_text_width(bar->win, label) + bar->padding;
    }
    return total;
}

static void bar_render(Bar *bar) {
    int cy = bar->height / 2 + ox_text_width(bar->win, "X") / 3;
    int w = bar_content_width(bar);
    ox_draw_rect(bar->win, 0, 0, w, bar->height, bar->bg);
    int cx = bar->padding;
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

static void date_update(void *ctx, char *buf, size_t len) {
    (void)ctx;
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    const char *wdays[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
    const char *mons[] = {"Jan","Feb","Mar","Apr","May","Jun",
                          "Jul","Aug","Sep","Oct","Nov","Dec"};
    snprintf(buf, len, "%s %s %d", wdays[tm->tm_wday], mons[tm->tm_mon], tm->tm_mday);
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
    static unsigned long long pt = 0, pi = 0;
    unsigned long long dt = total - pt, di = idle - pi;
    pt = total; pi = idle;
    int pct = (dt > 0) ? (int)((dt - di) * 100 / dt) : 0;
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
    snprintf(buf, len, "%ldMB", (total - avail) / 1024);
}

static void vol_update(void *ctx, char *buf, size_t len) {
    (void)ctx;
    FILE *f = popen("pactl get-sink-volume @DEFAULT_SINK@ 2>/dev/null | grep -oP '\\d+%' | head -1 | grep -oP '\\d+'", "r");
    if (!f) { snprintf(buf, len, "??%%"); return; }
    if (fgets(buf, len, f)) {
        size_t n = strlen(buf);
        while (n > 0 && (buf[n-1] == '\n' || buf[n-1] == '\r')) buf[--n] = '\0';
    } else {
        snprintf(buf, len, "??%%");
    }
    pclose(f);
}

static void bright_update(void *ctx, char *buf, size_t len) {
    (void)ctx;
    FILE *f = fopen("/sys/class/backlight/backlight/brightness", "r");
    if (!f) { snprintf(buf, len, "??%%"); return; }
    long cur = 0, max = 15360;
    fscanf(f, "%ld", &cur);
    fclose(f);
    snprintf(buf, len, "%ld%%", cur * 100 / max);
}

int main(void) {
    ox_init();
    int sw = DisplayWidth(ox_display(), ox_screen());
    int h = 24, pad = 8;
    const char *font = "monospace:size=11";

    /* left: time, cpu, mem */
    Bar left = { .height = h, .padding = pad, .fg = "#ffffff", .bg = "#000000", .sep = "#555555" };
    bar_add(&left, ox_widget_new("time", 1.0));
    ox_widget_set_icon(left.widgets[0], "TIME");
    ox_widget_set_update(left.widgets[0], time_update, NULL);

    bar_add(&left, ox_widget_new("cpu", 2.0));
    ox_widget_set_icon(left.widgets[1], "CPU");
    ox_widget_set_update(left.widgets[1], cpu_update, NULL);

    bar_add(&left, ox_widget_new("mem", 2.0));
    ox_widget_set_icon(left.widgets[2], "MEM");
    ox_widget_set_update(left.widgets[2], mem_update, NULL);

    /* center: date */
    Bar center = { .height = h, .padding = pad, .fg = "#ffffff", .bg = "#000000", .sep = "#555555" };
    bar_add(&center, ox_widget_new("date", 60.0));
    ox_widget_set_icon(center.widgets[0], "DATE");
    ox_widget_set_update(center.widgets[0], date_update, NULL);

    /* right: vol, bright */
    Bar right = { .height = h, .padding = pad, .fg = "#ffffff", .bg = "#000000", .sep = "#555555" };
    bar_add(&right, ox_widget_new("vol", 2.0));
    ox_widget_set_icon(right.widgets[0], "VOL");
    ox_widget_set_update(right.widgets[0], vol_update, NULL);

    bar_add(&right, ox_widget_new("bright", 5.0));
    ox_widget_set_icon(right.widgets[1], "LIT");
    ox_widget_set_update(right.widgets[1], bright_update, NULL);

    /* update all widgets to get initial content */
    for (int i = 0; i < left.count; i++) ox_widget_update(left.widgets[i]);
    for (int i = 0; i < center.count; i++) ox_widget_update(center.widgets[i]);
    for (int i = 0; i < right.count; i++) ox_widget_update(right.widgets[i]);

    /* measure content widths */
    /* create temp windows just for measuring */
    OxWindow *tmp = ox_window_new(-100, -100, 1, 1);
    ox_window_set_font(tmp, font);

    int lw = pad;
    for (int i = 0; i < left.count; i++) {
        if (i > 0) lw += ox_text_width(tmp, " | ") + pad;
        const char *icon = ox_widget_get_icon(left.widgets[i]);
        const char *label = ox_widget_get_label(left.widgets[i]);
        if (icon) lw += ox_text_width(tmp, icon);
        lw += ox_text_width(tmp, label) + pad;
    }

    int cw = pad;
    for (int i = 0; i < center.count; i++) {
        if (i > 0) cw += ox_text_width(tmp, " | ") + pad;
        const char *icon = ox_widget_get_icon(center.widgets[i]);
        const char *label = ox_widget_get_label(center.widgets[i]);
        if (icon) cw += ox_text_width(tmp, icon);
        cw += ox_text_width(tmp, label) + pad;
    }

    int rw = pad;
    for (int i = 0; i < right.count; i++) {
        if (i > 0) rw += ox_text_width(tmp, " | ") + pad;
        const char *icon = ox_widget_get_icon(right.widgets[i]);
        const char *label = ox_widget_get_label(right.widgets[i]);
        if (icon) rw += ox_text_width(tmp, icon);
        rw += ox_text_width(tmp, label) + pad;
    }

    ox_window_destroy(tmp);

    /* create windows at final positions */
    left.win = ox_window_new(0, 0, lw, h);
    ox_window_set_bg(left.win, left.bg);
    ox_window_set_font(left.win, font);
    ox_window_set_strut(left.win, h, 0, 0, 0);

    center.win = ox_window_new((sw - cw) / 2, 0, cw, h);
    ox_window_set_bg(center.win, center.bg);
    ox_window_set_font(center.win, font);

    right.win = ox_window_new(sw - rw, 0, rw, h);
    ox_window_set_bg(right.win, right.bg);
    ox_window_set_font(right.win, font);

    ox_window_show(left.win);
    ox_window_show(center.win);
    ox_window_show(right.win);

    Bar *bars[] = { &left, &center, &right };
    OxWidget *all[] = { left.widgets[0], left.widgets[1], left.widgets[2],
                        center.widgets[0], right.widgets[0], right.widgets[1] };
    int nw = 6;

    struct timespec ts = { .tv_sec = 0, .tv_nsec = 100000000 };
    struct timespec now_ts;
    clock_gettime(CLOCK_MONOTONIC, &now_ts);

    for (;;) {
        clock_gettime(CLOCK_MONOTONIC, &now_ts);
        double cur = now_ts.tv_sec + now_ts.tv_nsec / 1e9;

        for (int i = 0; i < nw; i++) {
            OxWidget *w = all[i];
            if (ox_widget_get_interval(w) > 0 &&
                cur - ox_widget_get_last_update(w) >= ox_widget_get_interval(w)) {
                ox_widget_update(w);
                ox_widget_set_last_update(w, cur);
            }
        }

        for (int i = 0; i < 3; i++) bar_render(bars[i]);

        Display *dpy = ox_display();
        while (XPending(dpy) > 0) { XEvent ev; XNextEvent(dpy, &ev); }
        XFlush(dpy);
        nanosleep(&ts, NULL);
    }

    return 0;
}
