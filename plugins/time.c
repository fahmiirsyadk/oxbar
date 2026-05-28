#include "oxbar.h"
#include "plugin.h"
#include <stdio.h>
#include <time.h>

static void update(void *ctx, char *buf, size_t len)
{
    (void)ctx;
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    snprintf(buf, len, "%02d:%02d:%02d",
        tm->tm_hour, tm->tm_min, tm->tm_sec);
}

static Widget *create(void) { return widget_create("time", NULL, 1.0, update, NULL); }
void plugin_init_time(void) { plugin_register("time", create); }
