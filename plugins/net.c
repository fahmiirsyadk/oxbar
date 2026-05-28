#include "oxbar.h"
#include "plugin.h"

static Widget *create(void) { return widget_create("net", NULL, 2.0, update_cmd, NULL); }
void plugin_init_net(void) { plugin_register("net", create); }
