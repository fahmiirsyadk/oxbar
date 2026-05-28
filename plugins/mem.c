#include "oxbar.h"
#include "plugin.h"

static Widget *create(void) { return widget_create("mem", NULL, 2.0, update_cmd, NULL); }
void plugin_init_mem(void) { plugin_register("mem", create); }
