#include "oxbar.h"
#include "plugin.h"

static Widget *create(void) { return widget_create("cpu", NULL, 2.0, update_cmd, NULL); }
void plugin_init_cpu(void) { plugin_register("cpu", create); }
