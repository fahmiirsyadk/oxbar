#include "oxbar.h"
#include "plugin.h"

static Widget *create(void) { return widget_create("load", NULL, 1.0, update_cmd, NULL); }
void plugin_init_load(void) { plugin_register("load", create); }
