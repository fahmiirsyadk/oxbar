#include "oxbar.h"
#include "plugin.h"

static Widget *create(void) { return widget_create("uptime", NULL, 60.0, update_cmd, NULL); }
void plugin_init_uptime(void) { plugin_register("uptime", create); }
