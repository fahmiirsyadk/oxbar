#include "oxbar.h"
#include "plugin.h"

static Widget *create(void) { return widget_create("vol", NULL, 2.0, update_cmd, NULL); }
void plugin_init_vol(void) { plugin_register("vol", create); }
