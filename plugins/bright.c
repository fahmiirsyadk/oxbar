#include "oxbar.h"
#include "plugin.h"

static Widget *create(void) { return widget_create("bright", NULL, 5.0, update_cmd, NULL); }
void plugin_init_bright(void) { plugin_register("bright", create); }
