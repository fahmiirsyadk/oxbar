#include "oxbar.h"
#include "plugin.h"

static Widget *create(void) { return widget_create("bat", NULL, 30.0, update_cmd, NULL); }
void plugin_init_bat(void) { plugin_register("bat", create); }
