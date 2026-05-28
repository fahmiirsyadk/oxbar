#include "oxbar.h"
#include "plugin.h"

static Widget *create(void) { return widget_create("disk", NULL, 30.0, update_cmd, NULL); }
void plugin_init_disk(void) { plugin_register("disk", create); }
