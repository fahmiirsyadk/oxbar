#define _POSIX_C_SOURCE 200809L
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include "ox.h"

static volatile int g_running = 1;

static void sig_handler(int sig) {
    (void)sig;
    g_running = 0;
}

void ox_quit(void) {
    g_running = 0;
}

void ox_run(void) {
    Display *dpy = ox_display();
    struct sigaction sa = { .sa_handler = sig_handler, .sa_flags = SA_RESTART };
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    struct timespec ts = { .tv_sec = 0, .tv_nsec = 100000000 };
    while (g_running) {
        while (XPending(dpy) > 0) {
            XEvent ev;
            XNextEvent(dpy, &ev);
        }
        XFlush(dpy);
        nanosleep(&ts, NULL);
    }
}
