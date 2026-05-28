#include <string.h>
#include "plugin.h"

#define MAX_PLUGINS 64

typedef struct {
    const char *name;
    WidgetCtor ctor;
} PluginEntry;

static PluginEntry plugins[MAX_PLUGINS];
static int plugin_count = 0;

void plugin_register(const char *name, WidgetCtor ctor)
{
    if (plugin_count < MAX_PLUGINS) {
        plugins[plugin_count].name = name;
        plugins[plugin_count].ctor = ctor;
        plugin_count++;
    }
}

WidgetCtor plugin_find(const char *name)
{
    for (int i = 0; i < plugin_count; i++) {
        if (strcmp(plugins[i].name, name) == 0)
            return plugins[i].ctor;
    }
    return NULL;
}

extern void plugin_init_time(void);
extern void plugin_init_cpu(void);
extern void plugin_init_mem(void);
extern void plugin_init_date(void);
extern void plugin_init_load(void);
extern void plugin_init_uptime(void);
extern void plugin_init_net(void);
extern void plugin_init_disk(void);
extern void plugin_init_vol(void);
extern void plugin_init_bright(void);
extern void plugin_init_bat(void);

void plugin_init_all(void)
{
    plugin_init_time();
    plugin_init_cpu();
    plugin_init_mem();
    plugin_init_date();
    plugin_init_load();
    plugin_init_uptime();
    plugin_init_net();
    plugin_init_disk();
    plugin_init_vol();
    plugin_init_bright();
    plugin_init_bat();
}
